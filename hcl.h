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
#include <boost/variant.hpp>
#include <string>
#include <map>

#define HCL_NS_BEGIN namespace hcl {

#define HCL_NS_END }

HCL_NS_BEGIN

template<class T> class NdArray;

typedef NdArray<unsigned char> NdArrayU8;
typedef NdArray<unsigned short> NdArrayU16;
typedef NdArray<unsigned int> NdArrayU32;
typedef NdArray<unsigned long long> NdArrayU64;

typedef NdArray<char> NdArrayS8;
typedef NdArray<short> NdArrayS16;
typedef NdArray<int> NdArrayS32;
typedef NdArray<long long> NdArrayS64;

typedef NdArray<float> NdArrayF32;
typedef NdArray<double> NdArrayF64;

class NdArrayBase {
public:
    typedef std::vector<unsigned int> DIMS_TYPE;
    typedef unsigned int SIZE_TYPE;
    typedef boost::shared_ptr<NdArrayBase> PTR_TYPE;

protected:
    DIMS_TYPE dims;
    SIZE_TYPE size;

public:
    virtual ~NdArrayBase() {}

    const DIMS_TYPE& getDims() const {
        return dims;
    }

    virtual const void* getRaw() const = 0;

    virtual const std::string& getName() const = 0;

    virtual const NdArrayU8 toU8() const = 0;
    virtual const NdArrayU16 toU16() const = 0;
    virtual const NdArrayU32 toU32() const = 0;
    virtual const NdArrayU64 toU64() const = 0;

    virtual const NdArrayS8 toS8() const = 0;
    virtual const NdArrayS16 toS16() const = 0;
    virtual const NdArrayS32 toS32() const = 0;
    virtual const NdArrayS64 toS64() const = 0;

    virtual const NdArrayF32 toF32() const = 0;
    virtual const NdArrayF64 toF64() const = 0;
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
        this->dims = dims;
    }

    NdArray(const boost::shared_ptr<T> &data, const DIMS_TYPE &dims) {
        size = 1;
        for (DIMS_TYPE::const_iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
        this->data = data;
        this->dims = dims;
    }

    NdArray (T *data, const DIMS_TYPE &dims) {
        size = 1;
        for (DIMS_TYPE::const_iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
        this->data = DATA_TYPE(data);
        this->dims = dims;
    }

    NdArray (const T *data, const DIMS_TYPE &dims) {
        size = 1;
        for (DIMS_TYPE::const_iterator it = dims.begin(); it != dims.end(); it++) size *= *it;
        this->data = DATA_TYPE(new T[size]);
        for (SIZE_TYPE i = 0; i < size; i++) this->data.get()[i] = data[i];
        this->dims = dims;
    }

    template<class U> const NdArray<U> to() const {
        if (dynamic_cast<const NdArray<U>*> (this)) return *((NdArray<U>*)(NdArrayBase*)this);
        NdArray<U> ary(dims);
        for (SIZE_TYPE i = 0; i < size; i++) ary.data.get()[i] = (U) data.get()[i];
        return ary;
    }

    const NdArrayU8 toU8() const { return to<unsigned char>(); }
    const NdArrayU16 toU16() const { return to<unsigned short>(); }
    const NdArrayU32 toU32() const { return to<unsigned int>(); }
    const NdArrayU64 toU64() const { return to<unsigned long long>(); }

    const NdArrayS8 toS8() const { return to<char>(); }
    const NdArrayS16 toS16() const { return to<short>(); }
    const NdArrayS32 toS32() const { return to<int>(); }
    const NdArrayS64 toS64() const { return to<long long>(); }

    const NdArrayF32 toF32() const { return to<float>(); }
    const NdArrayF64 toF64() const { return to<double>(); }

    friend NdArrayU8;
    friend NdArrayU16;
    friend NdArrayU32;
    friend NdArrayU64;

    friend NdArrayS8;
    friend NdArrayS16;
    friend NdArrayS32;
    friend NdArrayS64;

    friend NdArrayF32;
    friend NdArrayF64;

    const T* getData() const {
        return data.get();
    }

    const void* getRaw() const {
        return data.get();
    }

#ifndef HCL_BUILD
    const std::string& getName() const {
        return name;
    }
#else
    const std::string& getName() const;
#endif
};

template class NdArray<unsigned char>;
template class NdArray<unsigned short>;
template class NdArray<unsigned int>;
template class NdArray<unsigned long long>;

template class NdArray<char>;
template class NdArray<short>;
template class NdArray<int>;
template class NdArray<long long>;

template class NdArray<float>;
template class NdArray<double>;

class Data {
public:
    typedef boost::shared_ptr<char> BUFFER_TYPE;

    BUFFER_TYPE buffer;
    unsigned long length;
    unsigned long allocd;

    Data(): length(0), allocd(0) { }

    void append(const void *data, unsigned long size);
};

class Options {
public:
    typedef std::string KEY_TYPE;
    typedef boost::variant<std::string, int, double> VALUE_TYPE;
    typedef std::map<KEY_TYPE, VALUE_TYPE> MAP_TYPE;
protected:
    MAP_TYPE values;
public:
    virtual ~Options() {}
    void set(const KEY_TYPE &name, const VALUE_TYPE &val);
    const VALUE_TYPE& get(const KEY_TYPE &name) const;
};

class Algorithm {
public:
    typedef boost::shared_ptr<NdArrayBase> ARRAY_TYPE;

    virtual ~Algorithm() {}
    virtual Data compress(const NdArrayBase*, const Options *opts = 0) const = 0;
    virtual ARRAY_TYPE decompress(const char*, unsigned long, const NdArrayBase::DIMS_TYPE &dims) const = 0;
    virtual std::string mimeType() const = 0;
};

class Container {
public:
    typedef boost::shared_ptr<NdArrayBase> ARRAY_TYPE;

    Data compress(const NdArrayBase*, const std::string&, const Options *opts = 0) const;
    ARRAY_TYPE decompress(const char*);
};


HCL_NS_END

#endif
