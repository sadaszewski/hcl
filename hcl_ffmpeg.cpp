//
// Hyperdimensional Compression Library (HCL)
//
// Author: Stanislaw Adaszewski, 2014
// email: s.adaszewski@gmail.com
// http://algoholic.eu
//

#include <hcl_ffmpeg.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

HCL_NS_BEGIN

class InitLibAv {
public:
    InitLibAv() {
        printf("Registering AV...\n");
        av_register_all();
    }
};

static InitLibAv initLibAv;

struct MemStream {
    typedef boost::shared_ptr<char> DTYPE;
    DTYPE data;
    off_t allocd;
    off_t pos;
    off_t size;

    MemStream() {
        allocd = 0;
        pos = 0;
        size = 0;
    }

    ~MemStream() {
    }

    static int read(void *opaque, uint8_t *buf, int buf_size) {
        MemStream *s = (MemStream*) opaque;
        off_t sz = buf_size;
        if (s->pos + sz >= s->size) sz = s->size - s->pos;
        memcpy(buf, s->data.get() + s->pos, sz);
        s->pos += sz;
        return sz;
    }

    static int write(void *opaque, uint8_t *buf, int buf_size) {
        printf("write %d\n", buf_size);

        MemStream *s = (MemStream*) opaque;
        if (s->pos + buf_size >= s->allocd) {
            if (s->pos > s->allocd) {
                s->allocd = s->pos + buf_size * 2;
            } else {
                s->allocd = s->allocd * 2 + buf_size;
            }
            DTYPE tmp = s->data;
            s->data = DTYPE((char*) malloc(s->allocd));
            if (s->data == 0) throw std::runtime_error("Out of memory writing to MemStream");
            if (s->size) memcpy(s->data.get(), tmp.get(), s->size);
        }
        memcpy(s->data.get() + s->pos, buf, buf_size);
        s->pos += buf_size;
        if (s->pos > s->size) s->size = s->pos;
        return buf_size;
    }

    static int64_t seek(void *opaque, int64_t offset, int whence) {
        MemStream *s = (MemStream*) opaque;
        switch(whence) {
        case SEEK_SET: s->pos = offset; break;
        case SEEK_CUR: s->pos += offset; break;
        case SEEK_END: s->pos = s->size - offset; break;
        }
        return 0;
    }

};

void FfmpegOptions::setCodec(const std::string &codec) {
    values["codec"] = codec;
}

Data Ffmpeg::compress(const NdArrayBase *ary, const Options *opts) const {
    NdArrayU8 u8 = ary->toU8();

    AVCodec *c = avcodec_find_encoder_by_name(boost::get<std::string>(opts->get("codec")).c_str());
    if (!c) throw std::runtime_error("Couldn't get specified codec.");

    AVOutputFormat *fmt = av_guess_format("matroska", 0, 0);
    if (!fmt) throw std::runtime_error("Couldn't get MKV format");

    AVFormatContext *ctx = 0;
    avformat_alloc_output_context2(&ctx, fmt, 0, 0);
    if (ctx == 0) throw std::runtime_error("Couldn't alloc format context.");

    unsigned char *buf = (unsigned char*) malloc(8192);
    if (buf == 0) throw std::runtime_error("Couldn't allocate buf");
    MemStream ms;

    AVIOContext *io = avio_alloc_context(buf, 8192,  1, &ms, MemStream::read, MemStream::write, MemStream::seek);
    if (io == 0) throw std::runtime_error("Couldn't allocate io context");

    ctx->pb = io;

    AVStream *s = avformat_new_stream(ctx, c);
    if (s == 0) throw std::runtime_error("Couldn't create new stream");

    AVCodecContext *cc = s->codec;

    const NdArrayBase::DIMS_TYPE &dims(ary->getDims());

    avcodec_get_context_defaults3(cc, c);
    //avcodec_align_dimensions();
    cc->width = dims[0];
    cc->height = dims[1];
    AVPixelFormat pix_fmt = avcodec_find_best_pix_fmt_of_list(c->pix_fmts, AV_PIX_FMT_GRAY8, 0, 0);
    cc->pix_fmt = pix_fmt;
    cc->flags |= CODEC_FLAG_GLOBAL_HEADER;
    cc->time_base.num = 1;
    cc->time_base.den = 30;
    s->time_base = cc->time_base;

    printf("pix_fmt: %d, time_base: %d/%d\n", cc->pix_fmt, cc->time_base.num, cc->time_base.den);

    if (avcodec_open2(cc, c, 0) < 0) throw std::runtime_error("Couldn't open codec.");

    SwsContext *sws = sws_getContext(dims[0], dims[1], AV_PIX_FMT_GRAY8, dims[0], dims[1], cc->pix_fmt, SWS_BILINEAR, 0, 0, 0);
    if (sws == 0) throw std::runtime_error("Couldn't allocate SWS");

    avformat_write_header(ctx, 0);

    off_t count = 1;
    for (off_t i = 2; i < dims.size(); i++) count *= dims[i];

    AVFrame *inframe = avcodec_alloc_frame();
    avpicture_alloc((AVPicture*) inframe, AV_PIX_FMT_GRAY8, dims[0], dims[1]);

    AVFrame *outframe = avcodec_alloc_frame();
    avpicture_alloc((AVPicture*) outframe, cc->pix_fmt, dims[0], dims[1]);

    for (off_t i = 0; i < count; i++) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = 0;
        pkt.size = 0;

        avpicture_fill((AVPicture*) inframe, (const uint8_t*) u8.getRaw(), AV_PIX_FMT_GRAY8, dims[0], dims[1]);
        if (sws_scale(sws, inframe->data, inframe->linesize, 0, dims[1], outframe->data, outframe->linesize) < 0) throw std::runtime_error("Couldnt't swscale frame");

        outframe->pts = av_rescale_q(i, cc->time_base, s->time_base);

        int got_pkt;

        if (avcodec_encode_video2(cc, &pkt, outframe, &got_pkt) < 0) throw std::runtime_error("Couldn't encode frame");

        if (got_pkt) {
            if (cc->coded_frame->pts != AV_NOPTS_VALUE) {
                pkt.pts = av_rescale_q(cc->coded_frame->pts, cc->time_base, s->time_base);
                pkt.dts = pkt.pts;
            }
            pkt.stream_index = s->index;
            if (cc->coded_frame->key_frame) pkt.flags |= AV_PKT_FLAG_KEY;

            if (av_interleaved_write_frame(ctx, &pkt) != 0) throw std::runtime_error("Couldn't write pkt");
        }
    }

    av_write_trailer(ctx);

    Data ret;

    ret.buffer = ms.data;
    ret.allocd = ms.allocd;
    ret.length = ms.size;

    return ret;
}

Algorithm::ARRAY_TYPE Ffmpeg::decompress(const char *, unsigned long, const NdArrayBase::DIMS_TYPE &dims) const {
}

std::string Ffmpeg::mimeType() const {
    return "application/vnd.hcl.Ffmpeg";
}

HCL_NS_END
