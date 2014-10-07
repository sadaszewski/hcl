//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#define HCL_BUILD
#include <hcl.h>
#include <hcl_jpeg.h>
#include <hcl_ffmpeg.h>

#include <string.h>
#include <stdexcept>
#include <stdio.h>

HCL_NS_BEGIN

template<> const std::string NdArrayU8::name = "u8";
template<> const std::string NdArrayU16::name = "u16";
template<> const std::string NdArrayU32::name = "u32";
template<> const std::string NdArrayU64::name = "u64";

template<> const std::string NdArrayS8::name = "s8";
template<> const std::string NdArrayS16::name = "s16";
template<> const std::string NdArrayS32::name = "s32";
template<> const std::string NdArrayS64::name = "s64";

template<> const std::string NdArrayF32::name = "f32";
template<> const std::string NdArrayF64::name = "f64";

template<class T> const std::string& NdArray<T>::getName() const {
    return name;
}

static Ffmpeg ffmpeg;
static JpegSequence jpegSeq;

Data Container::compress(const NdArrayBase *ary, const std::string &method, const Options *opts) const {
    Algorithm *algo;

    if (method == "JpegSequence") algo = &jpegSeq;
    else if (method == "Ffmpeg") algo = &ffmpeg;
    else throw std::runtime_error("Unknown compression method.");

    Data compr = algo->compress(ary, opts);

    std::string mime = algo->mimeType();

    Data out;

    int version = 1;
    out.append(&version, sizeof(int));
    out.append(mime.c_str(), mime.size() + 1);
    out.append(ary->getName().c_str(), ary->getName().size() + 1);
    unsigned int ndims = ary->getDims().size();
    out.append(&ndims, sizeof(unsigned int));
    out.append(&ary->getDims()[0], ndims * sizeof(unsigned int));
    out.append(&compr.length, sizeof(unsigned int));
    out.append(compr.buffer.get(), compr.length);

    return out;
}

Container::ARRAY_TYPE Container::decompress(const char *ptr) {
    int version;
    memcpy(&version, ptr, sizeof(int)); ptr += sizeof(int);

    printf("Version: %d\n", version);

    char *tmp = strdup(ptr); std::string mime = tmp; free(tmp); ptr += mime.size() + 1;
    tmp = strdup(ptr); std::string name = tmp; free(tmp); ptr += name.size() + 1;

    printf("mime: %s\n", mime.c_str());
    printf("dtype: %s\n", name.c_str());

    unsigned int ndims;
    memcpy(&ndims, ptr, sizeof(unsigned int)); ptr += sizeof(unsigned int);
    NdArrayBase::DIMS_TYPE dims;
    for (unsigned int i = 0; i < ndims; i++) { dims.push_back(*((unsigned int*) ptr)); ptr += sizeof(unsigned int); }
    unsigned int len;
    memcpy(&len, ptr, sizeof(unsigned int)); ptr += sizeof(unsigned int);

    // printf("ndims: %u, len: %u, dims[0]: %u, dims[1]: %u, dims[2]: %u\n", ndims, len, dims[0], dims[1], dims[2]);

    Container::ARRAY_TYPE ary;

    if (name == "u8"); // ary = Container::ARRAY_TYPE(new NdArrayU8(dims));
    else if (name == "u16"); // ary = Container::ARRAY_TYPE(new NdArrayU16(dims));
    else if (name == "u32"); // ary = Container::ARRAY_TYPE(new NdArrayU32(dims));
    else if (name == "u64"); // ary = Container::ARRAY_TYPE(new NdArrayU64(dims));
    else if (name == "s8"); // ary = Container::ARRAY_TYPE(new NdArrayS8(dims));
    else if (name == "s16"); // ary = Container::ARRAY_TYPE(new NdArrayS16(dims));
    else if (name == "s32"); // ary = Container::ARRAY_TYPE(new NdArrayS32(dims));
    else if (name == "s64"); // ary = Container::ARRAY_TYPE(new NdArrayS64(dims));
    else if (name == "f32"); // ary = Container::ARRAY_TYPE(new NdArrayF32(dims));
    else if (name == "f64"); // ary = Container::ARRAY_TYPE(new NdArrayF64(dims));
    else throw std::runtime_error("Unknown data type.");

    if (mime == "application/vnd.hcl.JpegSequence") ary = JpegSequence().decompress(ptr, len, dims);
    else throw std::runtime_error("Unknown compression method.");

    return ary;
}

void Data::append(const void *data, unsigned long size) {
    if (!buffer.get()) {
        // printf("Allocating fresh buffer...\n");
        allocd = size * 2;
        buffer = Data::BUFFER_TYPE(new char[allocd]);
        if (buffer.get() == 0) throw std::runtime_error("Data::append() Out of memory");
    } else if (length + size >= allocd) {
        Data::BUFFER_TYPE old = buffer;
        allocd = allocd * 2 + size;
        buffer = Data::BUFFER_TYPE(new char[allocd]);
        if (buffer.get() == 0) throw std::runtime_error("Data::append() Out of memory");
        memcpy(buffer.get(), old.get(), length);
    }
    memcpy(buffer.get() + length, data, size);
    length += size;
}

void Options::set(const KEY_TYPE &name, const VALUE_TYPE &val) {
    values[name] = val;
}

const Options::VALUE_TYPE& Options::get(const KEY_TYPE &name) const {
    return values.find(name)->second;
}

HCL_NS_END
