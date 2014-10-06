#include <hcl_ffmpeg.h>

HCL_NS_BEGIN

void FfmpegOptions::setCodec(const std::string &codec) {
    values["codec"] = codec;
}

Data Ffmpeg::compress(const NdArrayBase *ary, const Options *opts) const {
}

Algorithm::ARRAY_TYPE Ffmpeg::decompress(const char *, unsigned long, const NdArrayBase::DIMS_TYPE &dims) const {
}

std::string Ffmpeg::mimeType() const {
    return "application/vnd.hcl.Ffmpeg";
}

HCL_NS_END
