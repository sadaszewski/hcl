//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#include <hcl.h>
#include <hcl_jpeg.h>

#include <string.h>
#include <stdexcept>

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

Data Container::compress(const NdArrayBase *ary, const std::string &method, const Options *opts) const {
    boost::shared_ptr<Algorithm> algo;

    if (method == "JpegSequence") {
            algo = boost::shared_ptr<Algorithm>(new JpegSequence());
    } else {
        throw std::runtime_error("Unknown compression method.");
    }

    Data compr = algo->compress(ary, opts);

    std::string mime = algo->mimeType();

    int version_size = sizeof(int);
    int mime_size = mime.size() + 1;
    int name_size = ary->getName().size() + 1;
    int dims_size = (ary->getDims().size() + 1) * sizeof(unsigned int);
    unsigned int compr_size = sizeof(unsigned int) + compr.length;
    unsigned int out_length = version_size + mime_size + name_size + dims_size + compr_size;

    Data out;
    out.buffer = Data::BUFFER_TYPE(new char[out_length]);

    char *ptr = out.buffer.get();
    int version = 1;
    memcpy(ptr, &version, sizeof(int)); ptr += sizeof(int);
    memcpy(ptr, mime.c_str(), mime_size); ptr += mime_size;
    memcpy(ptr, ary->getName().c_str(), name_size); ptr += name_size;
    unsigned int ndims = ary->getDims().size();
    memcpy(ptr, &ndims, sizeof(unsigned int)); ptr += sizeof(unsigned int);
    memcpy(ptr, &ary->getDims()[0], dims_size); ptr += dims_size;
    unsigned int len = compr.length;
    memcpy(ptr, &len, sizeof(unsigned int)); ptr += sizeof(unsigned int);
    memcpy(ptr, compr.buffer.get(), compr.length);

    return out;
}

Container::ARRAY_TYPE Container::decompress(const char *ptr) {
    int version;
    memcpy(&version, ptr, sizeof(int));

    char *tmp = strdup(ptr); std::string mime = tmp; free(tmp); ptr += mime.size() + 1;
    tmp = strdup(ptr); std::string name = tmp; free(tmp); ptr += name.size() + 1;

    unsigned int ndims;
    memcpy(&ndims, ptr, sizeof(unsigned int));
    NdArrayBase::DIMS_TYPE dims;
    for (unsigned int i = 0; i < ndims; i++) { dims.push_back(*((unsigned int*) ptr)); ptr += sizeof(unsigned int); }
    unsigned int len;
    memcpy(&len, ptr, sizeof(unsigned int));

    Container::ARRAY_TYPE ary;

    if (name == "u8") ary = Container::ARRAY_TYPE(new NdArrayU8(dims));
    else if (name == "u16") ary = Container::ARRAY_TYPE(new NdArrayU16(dims));
    else if (name == "u32") ary = Container::ARRAY_TYPE(new NdArrayU32(dims));
    else if (name == "u64") ary = Container::ARRAY_TYPE(new NdArrayU64(dims));
    else if (name == "s8") ary = Container::ARRAY_TYPE(new NdArrayS8(dims));
    else if (name == "s16") ary = Container::ARRAY_TYPE(new NdArrayS16(dims));
    else if (name == "s32") ary = Container::ARRAY_TYPE(new NdArrayS32(dims));
    else if (name == "s64") ary = Container::ARRAY_TYPE(new NdArrayS64(dims));
    else if (name == "f32") ary = Container::ARRAY_TYPE(new NdArrayF32(dims));
    else if (name == "f64") ary = Container::ARRAY_TYPE(new NdArrayF64(dims));
    else throw std::runtime_error("Unknown data type.");

    if (mime == "application/vnd.hcl.JpegSequence") JpegSequence().decompress(ptr, len, ary.get());
    else throw std::runtime_error("Unknown compression method.");

    return ary;
}

void Data::append(const char *data, unsigned long size) {
    if (!buffer.get()) {
        allocd = size * 2;
        buffer = Data::BUFFER_TYPE(new char[allocd]);
    } else if (length + size >= allocd) {
        Data::BUFFER_TYPE old = buffer;
        allocd = allocd * 2 + size;
        buffer = Data::BUFFER_TYPE(new char[allocd]);
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
