#if 0
gcc -g pyhcl.cpp -fPIC -shared -o pyhcl.so
exit
#endif

//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#include <python2.7/Python.h>
#include <python2.7/numpy/arrayobject.h>
#include <hcl.h>

static PyObject *error;

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
            opts->set(PyString_AsString(key), PyString_AsString(value));
        } else if (PyInt_Check(value)) {
            opts->set(PyString_AsString(key), PyInt_AsInt(value));
        } else if (PyFloat_Check(value)) {
            opts->set(PyString_AsString(key), PyFloat_AsDouble(value));
        } else {
            PyErr_Format(error, "opts values must be strings or int/float numbers");
            return 0;
        }
    }

    hcl::NdArrayBase::DIMS_TYPE dims;
    unsigned int ndims = PyArray_NDIMS(ary);
    for (unsigned int i = 0; i < ndims; i++) {
        dims.push_back(PyArray_DIM(ary, i));
    }

    boost::shared_ptr<hcl::NdArrayBase> ary;

#define CASE(a, b) case a: ary.reset(new NdArray<b>((const b*) PyArray_DATA(ary), dims));

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

    hcl::Container c;
    hcl::Data data;
    try {
        data = c.compress(ary, method, opts);
    } catch (std::runtime_error &e) {
        PyErr_Format(error, e.what());
        return 0;
    }

    PyObject *ret = PyString_FromStringAndSize(data.buffer.get(), data.length);

    return ret;
}

//
// Usage:
//
// ary = hcl_decompress(data)
//
static PyObject* hcl_decompress(PyObject *self, PyObject *args) {
    const char *data;
    Py_ssize_t length;

    if (!PyArg_ParseTuple(args, "s#", &data, &length)) {
        PyErr_Format(error, "Couldn't parse arguments");
        return 0;
    }

    boost::shared_ptr<hcl::NdArrayBase> ary;
    hcl::Container c;

    ary = c.decompress(data);
    int dtype;

#define CASE(a, b) if (dynamic_cast<NdArray<b> *>(ary.get())) dtype = a; else

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

    PyObject *ret = PyArray_SimpleNewFromData(ary->getDims().size(), &ary->getDims()[0], dtype, ary->getRaw());

    hcl::Data data;
    try {
        data = c.compress(ary, method, opts);
    } catch (std::runtime_error &e) {
        PyErr_Format(error, e.what());
        return 0;
    }

    PyObject *ret = PyString_FromStringAndSize(data.buffer.get(), data.length);

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

    error = PyErr_NewException("pyhcl.error", 0, 0);
    Py_INCREF(error);
    PyModule_AddObject(m, "error", error);

    import_array();
}
