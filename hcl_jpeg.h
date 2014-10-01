//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#ifndef _HCL_JPEG_H_
#define _HCL_JPEG_H_

#include <hcl.h>

HCL_NS_BEGIN

class JpegSequenceOptions: public Options {
protected:
    int q;

public:
    void setQuality(int q) { this->q = q; }
    int quality() const { return q; }
};

class JpegSequence: public Algorithm {
public:
    Data compress(const NdArrayBase *ary, const Options *opts = 0) const;
    void decompress(const char*, NdArrayBase *ary) const;
    std::string mimeType() const;
};

HCL_NS_END

#endif
