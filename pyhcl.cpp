#if 0
g++ -g  -fPIC -shared -o pyhcl.so -I. -L. -lhcl -I/usr/include/python2.7 -I/opt/local/include -I/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -I/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/numpy/core/include/  pyhcl.cpp -L/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib -lpython2.7
echo Done
exit
#endif

//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#include <Python.h>
#include <numpy/arrayobject.h>
#include <hcl.h>
#include <stdexcept>

static PyObject *error;

template<class T> struct PySharedPtr {
    PyObject_HEAD;
    boost::shared_ptr<T> *ptr;
};

template<class T> static void PySharedPtr_dealloc(PyObject *self) {
    delete ((PySharedPtr<T>*) self)->ptr;
}

#define SHARED_PTR_TYPE(a) PyTypeObject PySharedPtrType_##a = { \
    PyObject_HEAD_INIT(NULL) \
    0, \
    "PySharedPtr_"#a, \
    sizeof(PySharedPtr<a>), \
    0, \
    PySharedPtr_dealloc<a>, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    Py_TPFLAGS_DEFAULT, \
    "Shared pointer object for "#a \
};

typedef hcl::NdArrayBase NdArrayBase;
typedef hcl::Data Data;

SHARED_PTR_TYPE(NdArrayBase)
SHARED_PTR_TYPE(char)

class DecRefDeleter {
    PyObject *obj;
public:
    DecRefDeleter(PyObject *obj) {
        this->obj = obj;
        Py_INCREF(obj);
    }
    void operator()(void *) {
        Py_DECREF(obj);
    }
};

//
// Usage:
//
// hcl_compress(ary, 'jpeg', {'quality': 75})
//
static PyObject* hcl_compress(PyObject *self, PyObject *args) {
    PyObject *ary;
    const char *method;
    PyObject *dict;

    if (!PyArg_ParseTuple(args, "OsO", &ary, &method, &dict)) {
        PyErr_Format(error, "Couldn't parse arguments");
        return 0;
    }

    if (!PyArray_Check(ary)) {
        PyErr_Format(error, "ary must be a NumPy array");
        return 0;
    }

    if (!PyDict_Check(dict)) {
        PyErr_Format(error, "opts must be a dictionary");
        return 0;
    }

    hcl::Options opts;
    PyObject *items = PyDict_Items(dict);
    for (unsigned int i = 0; i < PyList_Size(items); i++) {
        PyObject *tuple = PyList_GetItem(items, i);
        PyObject *key = PyTuple_GetItem(tuple, 0);
        PyObject *value = PyTuple_GetItem(tuple, 1);
        if (!PyString_Check(key)) {
            PyErr_Format(error, "opts keys must be strings");
            return 0;
        }
        if (PyString_Check(value)) {
            opts.set(PyString_AsString(key), std::string(PyString_AsString(value)));
        } else if (PyInt_Check(value)) {
            opts.set(PyString_AsString(key), (int) PyInt_AsLong(value));
        } else if (PyFloat_Check(value)) {
            opts.set(PyString_AsString(key), PyFloat_AsDouble(value));
        } else {
            PyErr_Format(error, "opts values must be strings or int/float numbers");
            return 0;
        }
    }

    hcl::NdArrayBase::DIMS_TYPE dims;
    unsigned int ndims = PyArray_NDIM(ary);
    for (unsigned int i = 0; i < ndims; i++) {
        dims.push_back(PyArray_DIM(ary, i));
    }
    printf("ndims: %u, dims.size(): %u\n", ndims, dims.size());

    int npy_dtype = PyArray_TYPE(ary);

    boost::shared_ptr<hcl::NdArrayBase> ndary;

#define CASE(a, b) case a: ndary.reset(new hcl::NdArray<b>(boost::shared_ptr<b>((b*) PyArray_DATA(ary), DecRefDeleter(ary)), dims)); break;

    switch(npy_dtype) {
    CASE(NPY_DOUBLE, double)
    CASE(NPY_FLOAT, float)
    CASE(NPY_UINT8, unsigned char)
    CASE(NPY_INT8, char)
    CASE(NPY_UINT16, unsigned short)
    CASE(NPY_INT16, short)
    CASE(NPY_UINT32, unsigned int)
    CASE(NPY_INT32, int)
    CASE(NPY_UINT64, unsigned long long)
    CASE(NPY_INT64, long long)
    default:
        PyErr_Format(error, "Unsupported dtype: %d", npy_dtype);
        return 0;
    }

    printf("ndary->getDims().size(): %u\n", ndary->getDims().size());

#undef CASE

    hcl::Container c;
    hcl::Data data;
    try {
        data = c.compress(ndary.get(), method, &opts);
    } catch (std::runtime_error &e) {
        PyErr_Format(error, "%s", e.what());
        return 0;
    }

    printf("data.length: %u\n", data.length);

    PyObject *ret = PyArray_SimpleNewFromData(1, (long*) &data.length, NPY_UINT8, data.buffer.get());
    PyObject *base = (PyObject*) PyObject_New(PySharedPtr<char>, &PySharedPtrType_char);
    ((PySharedPtr<char>*) base)->ptr = new boost::shared_ptr<char>(data.buffer);
    PyArray_BASE(ret) = base;

    // PyObject *ret = PyString_FromStringAndSize(data.buffer.get(), data.length);

    return ret;
}

//
// Usage:
//
// ary = hcl_decompress(data)
//
static PyObject* hcl_decompress(PyObject *self, PyObject *args) {
    PyObject *data_ary;

    if (!PyArg_ParseTuple(args, "O", &data_ary)) {
        PyErr_Format(error, "Couldn't parse arguments");
        return 0;
    }

    if (!PyArray_Check(data_ary) || PyArray_NDIM(data_ary) != 1 || PyArray_TYPE(data_ary) != NPY_UINT8) {
        PyErr_Format(error, "ary must be a 1-dimensional unsigned char NumPy array");
        return 0;
    }

    const char *data = (const char*) PyArray_DATA(data_ary);
    Py_ssize_t length = PyArray_DIM(data_ary, 0);

    printf("data: 0x%08X, length: %u\n", data, length);

    boost::shared_ptr<hcl::NdArrayBase> ary;
    hcl::Container c;

    try {
        ary = c.decompress(data);
    } catch (std::runtime_error &e) {
        PyErr_Format(error, "%s", e.what());
        return 0;
    }
    int dtype;

#define CASE(a, b) if (dynamic_cast<hcl::NdArray<b> *>(ary.get())) dtype = a; else

    CASE(NPY_DOUBLE, double)
    CASE(NPY_FLOAT, float)
    CASE(NPY_UINT8, unsigned char)
    CASE(NPY_INT8, char)
    CASE(NPY_UINT16, unsigned short)
    CASE(NPY_INT16, short)
    CASE(NPY_UINT32, unsigned int)
    CASE(NPY_INT32, int)
    CASE(NPY_UINT64, unsigned long long)
    CASE(NPY_INT64, long long)
    /* else */ {
        PyErr_Format(error, "Unsupported dtype");
        return 0;
    }

    printf("dtype: %d\n", dtype);
    printf("ary->getDims().size(): %u\n", ary->getDims().size());
    printf("ary->getDims()[0]: %u\n", ary->getDims()[0]);
    printf("ary->getDims()[1]: %u\n", ary->getDims()[1]);
    printf("ary->getDims()[2]: %u\n", ary->getDims()[2]);
    printf("ary->getName(): %s\n", ary->getName().c_str());
    printf("ary->getRaw(): 0x%08X\n", ary->getRaw());

    std::vector<npy_intp> npy_dims(ary->getDims().size());
    for (int i = 0; i < ary->getDims().size(); i++) npy_dims[i] = ary->getDims()[i];

    PyObject *ret = PyArray_SimpleNewFromData(ary->getDims().size(), &npy_dims[0], dtype, (void*) ary->getRaw());

    if (PyArray_Check(ret)) {
        printf("array ok, %u, %u\n", sizeof(long), sizeof(int));
        printf("array dim: %u, %u, %u\n", PyArray_DIM(ret, 0), PyArray_DIM(ret, 1), PyArray_DIM(ret, 2));
    }

    printf("ret: 0x%08X\n", ret);

    PyObject *base = (PyObject*) PyObject_New(PySharedPtr<NdArrayBase>, &PySharedPtrType_NdArrayBase);
    ((PySharedPtr<NdArrayBase>*) base)->ptr = new boost::shared_ptr<NdArrayBase>(ary);
    PyArray_BASE(ret) = base;

    return ret;
}

static PyMethodDef pyhcl_methods[] = {
    {"hcl_compress", hcl_compress, METH_VARARGS, "Compress NumPy array."},
    {"hcl_decompress", hcl_decompress, METH_VARARGS, "Decompress NumPy array."},
    {0, 0, 0, 0}
};

PyMODINIT_FUNC initpyhcl() {


    PyObject *m;
    m = Py_InitModule("pyhcl", pyhcl_methods);
    if (!m) {
        return;
    }

    printf("initpyhcl()\n");

    PySharedPtrType_NdArrayBase.tp_new = PyType_GenericNew;
    PySharedPtrType_char.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PySharedPtrType_char) < 0 || PyType_Ready(&PySharedPtrType_NdArrayBase))
        return;

    error = PyErr_NewException("pyhcl.error", 0, 0);
    Py_INCREF(error);
    PyModule_AddObject(m, "error", error);

    import_array();
}
