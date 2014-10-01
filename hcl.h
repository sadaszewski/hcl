//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#ifndef _HCL_H_
#define _HCL_H_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <string>

#define HCL_NS_BEGIN namespace hcl {

#define HCL_NS_END }

HCL_NS_BEGIN

class NdArrayBase {
public:
    typedef std::vector<unsigned int> DIMS_TYPE;
    typedef unsigned int SIZE_TYPE;

protected:
    DIMS_TYPE dims;
    SIZE_TYPE size;

public:
    virtual ~NdArrayBase() {}

    const DIMS_TYPE getDims() const {
        return dims;
    }

    virtual std::string getName() const = 0;
};

template<class T> class NdArray: public NdArrayBase {
public:
    typedef boost::shared_ptr<T> DATA_TYPE;

protected:
    static const std::string name;
    DATA_TYPE data;

public:
    NdArray (const DIMS_TYPE &dims) {
        size = 1;
        for (DIMS_TYPE::const_iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
        data = DATA_TYPE(new T[size]);
    }

    NdArray (T *data, const DIMS_TYPE &dims) {
        size = 1;
        for (DIMS_TYPE::const_iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
        this->data = DATA_TYPE(data);
    }

    NdArray (const T *data, const DIMS_TYPE &dims) {
        size = 1;
        for (DIMS_TYPE::const_iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
        this->data = DATA_TYPE(new T[size]);
        for (SIZE_TYPE i = 0; i < size; i++) this->data.get()[i] = data[i];
    }

    template<class U> NdArray<U> to() {
        if (dynamic_cast<NdArray<U>*> (this)) return *this;
        NdArray<U> ary(dims);
        for (SIZE_TYPE i = 0; i < size; i++) ary.data[i] = (U) data[i];
        return ary;
    }

    const T* getData() const {
        return data.get();
    }

    std::string getName() const {
        return name;
    }
};

typedef NdArray<unsigned char> NdArrayU8; template class NdArray<unsigned char>;
typedef NdArray<unsigned short> NdArrayU16; template class NdArray<unsigned short>;
typedef NdArray<unsigned int> NdArrayU32; template class NdArray<unsigned int>;
typedef NdArray<unsigned long long> NdArrayU64; template class NdArray<unsigned long long>;

typedef NdArray<char> NdArrayS8; template class NdArray<char>;
typedef NdArray<short> NdArrayS16; template class NdArray<short>;
typedef NdArray<int> NdArrayS32; template class NdArray<int>;
typedef NdArray<long long> NdArrayS64; template class NdArray<long long>;

typedef NdArray<float> NdArrayF32; template class NdArray<float>;
typedef NdArray<double> NdArrayF64; template class NdArray<double>;

class Data {
public:
    typedef boost::shared_ptr<char> BUFFER_TYPE;

    BUFFER_TYPE buffer;
    int length;
};

class Options {
public:
    virtual ~Options() {}
};

class Algorithm {
public:
    typedef boost::shared_ptr<NdArrayBase> ARRAY_TYPE;

    virtual ~Algorithm() {}
    virtual Data compress(const NdArrayBase*, const Options *opts = 0) const = 0;
    virtual void decompress(const char*, NdArrayBase*) const = 0;
    virtual std::string mimeType() const = 0;
};

class Container {
public:
    typedef boost::shared_ptr<NdArrayBase> ARRAY_TYPE;

    Data compress(const NdArrayBase*, const Algorithm*, const Options *opts = 0) const;
    ARRAY_TYPE decompress(const char*);
};



HCL_NS_END

#endif
