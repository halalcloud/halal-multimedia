#include "bitstreamfilter.h"
#include <stdexcept>
#include <stdio.h>
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
        if (ptmp + nalSize > buf +size)
            break;
        memcpy(pData, ptmp, nalSize);
        pData += nalSize;
        ptmp += nalSize;
    }
    _cxt._used = pData - _cxt._data.get();
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
        buf_size = _cxt._used;
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
    av_free(ctx->spspps_buf);
    return 1;

fail:
    av_freep(poutbuf);
    *poutbuf_size = 0;
    av_free(ctx->spspps_buf);
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

aac_asc2adts::aac_asc2adts(uint8_t *buf, uint32_t size)
{
    assert(buf && size);
    _extra.reset(new uint8_t[size]);
    _extr_size = size;
    GetBitContext gb;
    PutBitContext pb;
    MPEG4AudioConfig m4ac;
    memset(&m4ac,0,sizeof(m4ac));
    int off;

    init_get_bits(&gb, buf, size * 8);
    if(0 <= (off = avpriv_mpeg4audio_get_config(&m4ac, buf, size * 8, 1)))
        ;//,E_INVALIDARG);

    skip_bits_long(&gb, off);


    memset(&_adts_cxt,0,sizeof(ADTSContext));
    _adts_cxt.objecttype        = m4ac.object_type - 1;
    _adts_cxt.sample_rate_index = m4ac.sampling_index;
    _adts_cxt.channel_conf      = m4ac.chan_config;

    if (3U < _adts_cxt.objecttype)
        throw std::runtime_error("MPEG-4 AOT is not allowed in ADTS");//,_adts_cxt.objecttype+1;
    if (15 == _adts_cxt.sample_rate_index)
        throw std::runtime_error("Escape sample rate index illegal in ADTS");
    if (0 != get_bits(&gb, 1))
        throw std::runtime_error("960/120 MDCT window is not allowed in ADTS");
    if (0 != get_bits(&gb, 1))
        throw std::runtime_error("Scalable configurations are not allowed in ADTS");
    if (0 != get_bits(&gb, 1))
        throw std::runtime_error("Extension flag is not allowed in ADTS");

    if (!_adts_cxt.channel_conf) {
        init_put_bits(&pb, _adts_cxt.pce_data, MAX_PCE_SIZE);

        put_bits(&pb, 3, 5); //ID_PCE
        _adts_cxt.pce_size = (avpriv_copy_pce_data(&pb, &gb) + 3) / 8;
        flush_put_bits(&pb);
    }

    _adts_cxt.write_adts = 1;
}

size_t aac_asc2adts::aac_asc2adts_filter(uint8_t *in, size_t insize, uint8_t **buf, size_t *size)
{
    *size = (unsigned)ADTS_HEADER_SIZE + _adts_cxt.pce_size + insize;
    *buf = new uint8_t[*size];
    uint32_t  sz = adts_write_frame_header(*buf,*size);
    memcpy(*buf +sz, in,insize);
    return 0;
}

size_t aac_asc2adts::adts_write_frame_header(uint8_t *buf, size_t size)
{
    PutBitContext pb;
    init_put_bits(&pb, buf, ADTS_HEADER_SIZE);

    /* adts_fixed_header */
    put_bits(&pb, 12, 0xfff);   /* syncword */
    put_bits(&pb, 1, 0);        /* ID */
    put_bits(&pb, 2, 0);        /* layer */
    put_bits(&pb, 1, 1);        /* protection_absent */

    put_bits(&pb, 2, _adts_cxt.objecttype); /* profile_objecttype */
    put_bits(&pb, 4, _adts_cxt.sample_rate_index);
    put_bits(&pb, 1, 0);        /* private_bit */
    put_bits(&pb, 3, _adts_cxt.channel_conf); /* channel_configuration */
    put_bits(&pb, 1, 0);        /* original_copy */
    put_bits(&pb, 1, 0);        /* home */
    /* adts_variable_header */
    put_bits(&pb, 1, 0);        /* copyright_identification_bit */
    put_bits(&pb, 1, 0);        /* copyright_identification_start */
    put_bits(&pb, 13, size); /* aac_frame_length */
    put_bits(&pb, 11, 0x7ff);   /* adts_buffer_fullness */
    put_bits(&pb, 2, 0);        /* number_of_raw_data_blocks_in_frame */
    flush_put_bits(&pb);

    if(0 < _adts_cxt.pce_size)
    {
        memcpy(buf+ADTS_HEADER_SIZE,_adts_cxt.pce_data,_adts_cxt.pce_size);
    }
    return ADTS_HEADER_SIZE + _adts_cxt.pce_size;

}

int parseAvcDecodeConfigRecord(uint8_t *p, uint32_t size, uint32_t &w, uint32_t &h)
{
    static int const NALOffset = 6;
    static int const sizeofNal = 2;
    static int const sizeofNalType = 1;
    h264_decode_t dec;
    if (p + NALOffset +sizeofNal + sizeofNalType>= p+size)
        return -1;
    uint16_t nalLen = p[NALOffset] << 8;
    nalLen += p[NALOffset+1];
    uint8_t type = p[NALOffset+2] & 0x1F;
    if (type == 7)
    {
        if (p+NALOffset+nalLen+sizeofNal+sizeofNalType >= p + size)
            return -1;
        h264_parse_sequence_parameter_set(&dec, p+NALOffset+sizeofNal, size - NALOffset - sizeofNal);
        w = dec.pic_width;
        h = dec.pic_height;
        return 0;
    }
    return -2;
}
