#include "bitstreamfilter.h"
#define AV_INPUT_BUFFER_PADDING_SIZE 32

#define AV_LOG_QUIET    -8
#define AV_LOG_PANIC     0
#define AV_LOG_FATAL     8
#define AV_LOG_ERROR    16
#define AV_LOG_WARNING  24
#define AV_LOG_INFO     32
#define AV_LOG_VERBOSE  40
#define AV_LOG_DEBUG    48
#define AV_LOG_TRACE    56

#define AVERROR(e) (e)


#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */
#define INT_MAX std::numeric_limits<int>::max()
h264_mp4toannexb::h264_mp4toannexb(unsigned char* extra_data, unsigned int extra_size)
{
    if (extra_size < 6 || !extra_data)
        assert(0);
    //_extra_data = new uint8_t[extra_size];
    //memcpy(_extra_data, extra_data, extra_size);
    _extra_data.reset(new uint8_t[extra_size]);
    memcpy(_extra_data.get(), extra_data, extra_size);
    _extr_size = extra_size;
    _cxt._data_size = 1 * 1024 * 1024;
    _cxt._used = 0;
    _cxt._data.reset(new uint8_t[_cxt._data_size]);

}
void h264_mp4toannexb::toStand(const uint8_t* buf, uint32_t size)
{
    if (size > _cxt._data_size)
    {
        _cxt._data_size = size;
        _cxt._data.reset(new uint8_t[_cxt._data_size]);
        printf("warning: video size greater than 1MB, may be error!\n");
    }
    const uint8_t* ptmp = buf;
    uint8_t* pData = _cxt._data.get();
    static const int STARTCODE = 4;
    while(ptmp < buf + size)
    {
        // get size
        uint32_t nalSize;
        uint8_t* pNalSize = (uint8_t*)&nalSize;
        pNalSize[3] = ptmp[0];
        pNalSize[2] = ptmp[1];
        pNalSize[1] = ptmp[2];
        pNalSize[0] = ptmp[3];

        // move pointer
        ptmp += sizeof(uint32_t);
        ptmp += STARTCODE;
        nalSize -= STARTCODE;

        // write size
        pData[0] = pNalSize[3];
        pData[1] = pNalSize[2];
        pData[2] = pNalSize[1];
        pData[3] = pNalSize[0];
        pData += 4;
        // nal
        memcpy(pData, ptmp, nalSize);
        pData += nalSize;
        ptmp += nalSize;
    }
}
int h264_mp4toannexb::h264_mp4toannexb_filter(uint8_t **poutbuf, int *poutbuf_size,
                            const uint8_t *buf, int buf_size, const char *args /*= NULL*/)
{
    if (buf_size < 5)
        return -1;
    if (buf[4] == 0
            && buf[5] == 0
            && buf[6] == 0
            && buf[7] == 1)
    {
        toStand(buf, buf_size);
        buf = _cxt._data.get();
        buf_size = _cxt._data_size;
    }
    std::auto_ptr<AVCodecContext> avctx(new AVCodecContext);
    avctx->extradata = _extra_data.get();
    avctx->extradata_size = _extr_size;
    std::auto_ptr<H264BSFContext> ctx(new H264BSFContext);
    memset(ctx.get(), 0, sizeof(H264BSFContext));
    //ctx->extradata_parsed = 0;
    int i;
    uint8_t unit_type;
    int32_t nal_size;
    uint32_t cumul_size    = 0;
    const uint8_t *buf_end = buf + buf_size;
    int ret = 0;

    /* nothing to filter */
    if (_extr_size < 6) {
        *poutbuf      = (uint8_t *)buf;
        *poutbuf_size = buf_size;
        return 0;
    }

    /* retrieve sps and pps NAL units from extradata */
    if (!ctx->extradata_parsed) {
        if (args && strstr(args, "private_spspps_buf"))
            ctx->private_spspps = 1;

        ret = h264_extradata_to_annexb(ctx.get(), avctx.get(), AV_INPUT_BUFFER_PADDING_SIZE);
        if (ret < 0)
            return ret;
        ctx->length_size      = ret;
        ctx->new_idr          = 1;
        ctx->idr_sps_seen     = 0;
        ctx->idr_pps_seen     = 0;
        ctx->extradata_parsed = 1;
    }

    *poutbuf_size = 0;
    *poutbuf      = NULL;
    do {
        ret= AVERROR(EINVAL);
        if (buf + ctx->length_size > buf_end)
            goto fail;

        for (nal_size = 0, i = 0; i<ctx->length_size; i++)
            nal_size = (nal_size << 8) | buf[i];

        buf      += ctx->length_size;
        unit_type = *buf & 0x1f;

        if (nal_size > buf_end - buf || nal_size < 0)
            goto fail;

        if (unit_type == 7)
            ctx->idr_sps_seen = ctx->new_idr = 1;
        else if (unit_type == 8) {
            ctx->idr_pps_seen = ctx->new_idr = 1;
            /* if SPS has not been seen yet, prepend the AVCC one to PPS */
            if (!ctx->idr_sps_seen) {
                if (ctx->sps_offset == -1)
                    av_log(avctx.get(), AV_LOG_WARNING, "SPS not present in the stream, nor in AVCC, stream may be unreadable\n");
                else {
                    if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                              ctx->spspps_buf + ctx->sps_offset,
                                              ctx->pps_offset != -1 ? ctx->pps_offset : ctx->spspps_size - ctx->sps_offset,
                                              buf, nal_size)) < 0)
                        goto fail;
                    ctx->idr_sps_seen = 1;
                    goto next_nal;
                }
            }
        }

        /* if this is a new IDR picture following an IDR picture, reset the idr flag.
         * Just check first_mb_in_slice to be 0 as this is the simplest solution.
         * This could be checking idr_pic_id instead, but would complexify the parsing. */
        if (!ctx->new_idr && unit_type == 5 && (buf[1] & 0x80))
            ctx->new_idr = 1;

        /* prepend only to the first type 5 NAL unit of an IDR picture, if no sps/pps are already present */
        if (ctx->new_idr && unit_type == 5 && !ctx->idr_sps_seen && !ctx->idr_pps_seen) {
            if ((ret=alloc_and_copy(poutbuf, poutbuf_size,
                                    ctx->spspps_buf, ctx->spspps_size,
                                    buf, nal_size)) < 0)
                goto fail;
            ctx->new_idr = 0;
            /* if only SPS has been seen, also insert PPS */
        } else if (ctx->new_idr && unit_type == 5 && ctx->idr_sps_seen && !ctx->idr_pps_seen) {
            if (ctx->pps_offset == -1) {
                av_log(avctx.get(), AV_LOG_WARNING, "PPS not present in the stream, nor in AVCC, stream may be unreadable\n");
                if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                          NULL, 0, buf, nal_size)) < 0)
                    goto fail;
            } else if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                             ctx->spspps_buf + ctx->pps_offset, ctx->spspps_size - ctx->pps_offset,
                                             buf, nal_size)) < 0)
                goto fail;
        } else {
            if ((ret=alloc_and_copy(poutbuf, poutbuf_size,
                                    NULL, 0, buf, nal_size)) < 0)
                goto fail;
            if (!ctx->new_idr && unit_type == 1) {
                ctx->new_idr = 1;
                ctx->idr_sps_seen = 0;
                ctx->idr_pps_seen = 0;
            }
        }

next_nal:
        buf        += nal_size;
        cumul_size += nal_size + ctx->length_size;
    } while (cumul_size < buf_size);

    return 1;

fail:
    av_freep(poutbuf);
    *poutbuf_size = 0;
    return ret;
}
int h264_mp4toannexb::h264_extradata_to_annexb(H264BSFContext *ctx, AVCodecContext *avctx, const int padding)
{
    uint16_t unit_size;
    uint64_t total_size                 = 0;
    uint8_t *out                        = NULL, unit_nb, sps_done = 0,
            sps_seen                   = 0, pps_seen = 0;
    const uint8_t *extradata            = avctx->extradata + 4;
    static const uint8_t nalu_header[4] = { 0, 0, 0, 1 };
    int length_size = (*extradata++ & 0x3) + 1; // retrieve length coded size

    ctx->sps_offset = ctx->pps_offset = -1;

    /* retrieve sps and pps unit(s) */
    unit_nb = *extradata++ & 0x1f; /* number of sps unit(s) */
    if (!unit_nb) {
        goto pps;
    } else {
        ctx->sps_offset = 0;
        sps_seen = 1;
    }

    while (unit_nb--) {
        int err;

        unit_size   = av_bswap16(*(uint16_t*)extradata);
        total_size += unit_size + 4;
        if (total_size > INT_MAX - padding) {
            av_log(avctx, AV_LOG_ERROR,
                   "Too big extradata size, corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if (extradata + 2 + unit_size > avctx->extradata + avctx->extradata_size) {
            av_log(avctx, AV_LOG_ERROR, "Packet header is not contained in global extradata, "
                                        "corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if ((err = av_reallocp(&out, total_size + padding)) < 0)
            return err;
        memcpy(out + total_size - unit_size - 4, nalu_header, 4);
        memcpy(out + total_size - unit_size, extradata + 2, unit_size);
        extradata += 2 + unit_size;
pps:
        if (!unit_nb && !sps_done++) {
            unit_nb = *extradata++; /* number of pps unit(s) */
            if (unit_nb) {
                ctx->pps_offset = total_size;
                pps_seen = 1;
            }
        }
    }

    if (out)
        memset(out + total_size, 0, padding);

    if (!sps_seen)
        av_log(avctx, AV_LOG_WARNING,
               "Warning: SPS NALU missing or invalid. "
               "The resulting stream may not play.\n");

    if (!pps_seen)
        av_log(avctx, AV_LOG_WARNING,
               "Warning: PPS NALU missing or invalid. "
               "The resulting stream may not play.\n");
    // wzd {
    //        if (!ctx->private_spspps) {
    //            av_free(avctx->extradata);
    //            avctx->extradata      = out;
    //            avctx->extradata_size = total_size;
    //        }
    // }
    ctx->spspps_buf  = out;
    ctx->spspps_size = total_size;

    return length_size;
}

int h264_mp4toannexb::alloc_and_copy(uint8_t **poutbuf, int *poutbuf_size,
                          const uint8_t *sps_pps, uint32_t sps_pps_size,
                          const uint8_t *in, uint32_t in_size)
{
    uint32_t offset         = *poutbuf_size;
    uint8_t nal_header_size = offset ? 3 : 4;
    int err;

    *poutbuf_size += sps_pps_size + in_size + nal_header_size;
    if ((err = av_reallocp(poutbuf,
                           *poutbuf_size + AV_INPUT_BUFFER_PADDING_SIZE)) < 0) {
        *poutbuf_size = 0;
        return err;
    }
    if (sps_pps)
        memcpy(*poutbuf + offset, sps_pps, sps_pps_size);
    memcpy(*poutbuf + sps_pps_size + nal_header_size + offset, in, in_size);
    if (!offset) {
        ((uint8_t*)(*poutbuf + sps_pps_size))[0] = 0;
        ((uint8_t*)(*poutbuf + sps_pps_size))[1] = 0;
        ((uint8_t*)(*poutbuf + sps_pps_size))[2] = 0;
        ((uint8_t*)(*poutbuf + sps_pps_size))[3] = 1;
    } else {
        (*poutbuf + offset + sps_pps_size)[0] =
                (*poutbuf + offset + sps_pps_size)[1] = 0;
        (*poutbuf + offset + sps_pps_size)[2] = 1;
    }

    return 0;
}

void *h264_mp4toannexb::av_realloc(void *ptr, size_t size)
{
#if CONFIG_MEMALIGN_HACK
    int diff;
#endif
static size_t max_alloc_size= INT_MAX;
    /* let's disallow possibly ambiguous cases */
    if (size > (max_alloc_size - 32))
        return NULL;

#if CONFIG_MEMALIGN_HACK
    //FIXME this isn't aligned correctly, though it probably isn't needed
    if (!ptr)
        return av_malloc(size);
    diff = ((char *)ptr)[-1];
    av_assert0(diff>0 && diff<=ALIGN);
    ptr = realloc((char *)ptr - diff, size + diff);
    if (ptr)
        ptr = (char *)ptr + diff;
    return ptr;
#elif HAVE_ALIGNED_MALLOC
    return _aligned_realloc(ptr, size + !size, ALIGN);
#else
    return realloc(ptr, size + !size);
#endif
}

int h264_mp4toannexb::av_reallocp(void *ptr, size_t size)
{
    void *val;

    if (!size) {
        av_freep(ptr);
        return 0;
    }

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc(val, size);

    if (!val) {
        av_freep(ptr);
        return AVERROR(ENOMEM);
    }

    memcpy(ptr, &val, sizeof(val));
    return 0;
}
