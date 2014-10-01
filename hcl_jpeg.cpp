//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#include <hcl_jpeg.h>

HCL_NS_BEGIN

Data JpegSequence::compress(const NdArrayBase *ary, const Options *opts) const {
}

void JpegSequence::decompress(const char*, NdArrayBase *ary) const {
}

std::string JpegSequence::mimeType() const {
    return "application/vnd.hcl.JpegSequence";
}


HCL_NS_END
