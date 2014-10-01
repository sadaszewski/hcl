//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#ifndef _HCL_H_
#define _HCL_H_

#include <vector>
#include <boost/shared_ptr.hpp>

#define HCL_NS hcl

#ifdef HCL_NS
namespace HCL_NS {
#endif

class NdArrayBase {
public:
    virtual ~NdArrayBase() {}
};

template<class T> class NdArray: public NdArrayBase {
public:
    typedef boost::shared_ptr<T> DATA_TYPE;
    typedef std::vector<unsigned int> DIMS_TYPE;
    typedef unsigned int SIZE_TYPE;

protected:
    DATA_TYPE data;
    DIMS_TYPE dims;
    SIZE_TYPE size;

public:
    NdArray (const DIMS_TYPE &dims) {
	size = 1;
	for (DIMS_TYPE::iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
	data = DATA_TYPE(new T[size]);
    }

    NdArray (T *data, const DIMS_TYPE &dims) {
	size = 1;
	for (DIMS_TYPE::iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
	this->data = DATA_TYPE(data);
    }

    NdArray (const T *data, const DIMS_TYPE &dims) {
	size = 1;
	for (DIMS_TYPE::iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
	this->data = DATA_TYPE(new T[size]);
	for (SIZE_TYPE i = 0; i < size; i++) this->data[i] = data[i];
    }

    template<class U> NdArray<U> to() {
	if (dynamic_cast<NdArray<U>*> (this)) return *this;
	NdArray<U> ary(dims);
	for (SIZE_TYPE i = 0; i < size; i++) ary.data[i] = (U) data[i];
	return ary;
    }
};

typedef NdArray<unsigned char> NdArrayU8; template class NdArrayU8;
typedef NdArray<unsigned short> NdArrayU16; template class NdArrayU16;
typedef NdArray<unsigned int> NdArrayU32; template class NdArrayU32;
typedef NdArray<unsigned long long> NdArrayU64; template class NdArrayU64;

typedef NdArray<char> NdArrayS8; template class NdArrayS8;
typedef NdArray<short> NdArrayS16; template class NdArrayS16;
typedef NdArray<int> NdArrayS32; template class NdArrayS32;
typedef NdArray<long long> NdArrayS64; template class NdArrayS64;

typedef NdArray<float> NdArrayF32; template class NdArrayF32;
typedef NdArray<double> NdArrayF64; template class NdArrayF64;

class Algorithm {
public:
    virtual ~Algorithm() {}
    char* compress(const NdArrayBase*, const Options *opts = 0);
    NdArray decompress(char*);
};

class Options {
public:
    virtual ~Options() {}
};

#ifdef HCL_NS
}
#endif

#endif
