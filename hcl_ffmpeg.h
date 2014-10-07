//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#ifndef _HCL_FFMPEG_H_
#define _HCL_FFMPEG_H_

#include <hcl.h>

HCL_NS_BEGIN

class FfmpegOptions: public Options {
public:
    void setCodec(const std::string&);
};

class Ffmpeg: public Algorithm {
public:
    Data compress(const NdArrayBase *ary, const Options *opts = 0) const;
    ARRAY_TYPE decompress(const char *, unsigned long, const NdArrayBase::DIMS_TYPE &dims) const;
    std::string mimeType() const;
};

HCL_NS_END

#endif
