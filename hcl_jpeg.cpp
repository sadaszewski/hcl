//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#include <hcl_jpeg.h>
#include <jpeglib.h>
#include <boost/fusion/include/all.hpp>

HCL_NS_BEGIN

template<class T> struct greater_than { T val; greater_than(const T &val) { this->val = val; } bool operator()(const T &x) const { return (x > val); } };

Data JpegSequence::compress(const NdArrayBase *ary, const Options *opts) const {
    assert(ary->getDims().size() >= 3);

    for (unsigned int i = 0; i < ary->getDims().size(); i++) {
        assert(ary->getDims()[i] > 0);
    }

    jpeg_compress_struct cinfo;
    jpeg_create_compress(&cinfo);
    unsigned char *out = 0;
    unsigned long outsize;
    jpeg_mem_dest(&cinfo, &out, &outsize);

    NdArrayU8 u8 = ary->toU8();

    cinfo.image_width = u8.getDims()[0];
    cinfo.image_height = u8.getDims()[1];
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;

    jpeg_set_defaults(&cinfo);
    if (opts) {
        jpeg_set_quality(&cinfo, boost::get<int>(opts->get("quality")), true);
    }

    const unsigned char *ptr = u8.getData();
    // unsigned int stride = cinfo.image_width * cinfo.image_height;

    const NdArrayBase::DIMS_TYPE &dims(u8.getDims());
    unsigned int count = 1;
    for (unsigned int i = 2; i < dims.size(); i++) count *= dims[i];

    Data data;

    for (unsigned int i = 0; i < count; i++) {
        jpeg_start_compress(&cinfo, true);
        while (cinfo.next_scanline < cinfo.image_height) {
            JSAMPROW row = (JSAMPROW) ptr;
            jpeg_write_scanlines(&cinfo, &row, 1);
            ptr += cinfo.image_width;
        }
        jpeg_finish_compress(&cinfo);
        data.append((const char*) out, outsize);
        free(out);
    }

    jpeg_destroy_compress(&cinfo);

    return data;
}

void JpegSequence::decompress(const char *data, unsigned long len, NdArrayBase *ary) const {
}

std::string JpegSequence::mimeType() const {
    return "application/vnd.hcl.JpegSequence";
}


HCL_NS_END
