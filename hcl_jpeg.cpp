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
#include <stdio.h>

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
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

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
        printf("i: %d\n", i);
        jpeg_start_compress(&cinfo, true);
        while (cinfo.next_scanline < cinfo.image_height) {
            JSAMPROW row = (JSAMPROW) ptr;
            jpeg_write_scanlines(&cinfo, &row, 1);
            ptr += cinfo.image_width;
        }
        jpeg_finish_compress(&cinfo);
        printf("outsize: %u\n", outsize);
        data.append((const char*) out, outsize);
        //free(out);
        //out = 0;
        //jpeg_mem_dest(&cinfo, &out, &outsize);
    }

    free(out);

    printf("data.length: %d\n", data.length);

    jpeg_destroy_compress(&cinfo);

    return data;
}

Algorithm::ARRAY_TYPE JpegSequence::decompress(const char *data, unsigned long len, const NdArrayBase::DIMS_TYPE &dims) const {
    assert(dims.size() >= 3);

    unsigned int count = 1;
    for (unsigned int i = 2; i < dims.size(); i++) count *= dims[i];

    printf("dims.size(): %u, count: %u\n", dims.size(), count);

    jpeg_decompress_struct cinfo;
    jpeg_create_decompress(&cinfo);

    // jpeg_set_defaults(&cinfo);
    jpeg_mem_src(&cinfo, (unsigned char*) data, len);

    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

    ARRAY_TYPE ret = ARRAY_TYPE(new NdArrayU8(dims));

    const char *ptr = (const char*) ret->getRaw();

    printf("ptr: 0x%08X\n", ptr);

    for (unsigned int i = 0; i < count; i++) {
        jpeg_read_header(&cinfo, true);

        printf("image_width: %u, image_height: %u\n", cinfo.image_width, cinfo.image_height);

        assert(cinfo.image_width == dims[0]);
        assert(cinfo.image_height == dims[1]);
        assert(cinfo.num_components == 1);
        assert(cinfo.jpeg_color_space == JCS_GRAYSCALE);

        jpeg_start_decompress(&cinfo);
        for (unsigned int k = 0; k < cinfo.image_height; k++) {
            JSAMPROW row = (JSAMPROW) ptr;
            jpeg_read_scanlines(&cinfo, &row, 1);
            ptr += cinfo.image_width;
        }
        jpeg_finish_decompress(&cinfo);
    }

    jpeg_destroy_decompress(&cinfo);

    return ret;
}

std::string JpegSequence::mimeType() const {
    return "application/vnd.hcl.JpegSequence";
}


HCL_NS_END
