#ifndef BITSTREAMFILTER_H
#define BITSTREAMFILTER_H
#include <memory>
#include <cassert>
#include <string.h>
#include <limits>
#include <stdint.h>
#include <stdlib.h>
class h264_mp4toannexb
{
    typedef struct _StandardContext
    {
        std::auto_ptr<uint8_t> _data; // for non-standard stream
        uint32_t _data_size;
        uint32_t _used;
    }StandardContext;
    std::auto_ptr<uint8_t>  _extra_data;
    unsigned int _extr_size;
    typedef struct H264BSFContext {
        int32_t  sps_offset;
        int32_t  pps_offset;
        uint8_t  length_size;
        uint8_t  new_idr;
        uint8_t  idr_sps_seen;
        uint8_t  idr_pps_seen;
        int      extradata_parsed;

        /* When private_spspps is zero then spspps_buf points to global extradata
           and bsf does replace a global extradata to own-allocated version (default
           behaviour).
           When private_spspps is non-zero the bsf uses a private version of spspps buf.
           This mode necessary when bsf uses in decoder, else bsf has issues after
           decoder re-initialization. Use the "private_spspps_buf" argument to
           activate this mode.
         */
        int      private_spspps;
        uint8_t *spspps_buf;
        uint32_t spspps_size;
    } H264BSFContext;
    typedef struct AVCodecContext
    {
        uint8_t* extradata;
        uint32_t extradata_size;
    } AVCodecContext;
StandardContext _cxt;
public:
    /**
     * @brief 构造函数
     * @param extra_data　全局头数据区
     * @param extra_size　全局头大小
     * @note 数据会复制到类内部，由类自己维护。
     */
    h264_mp4toannexb(unsigned char* extra_data, unsigned int extra_size);

    /**
     * @brief 把mp4格式的h264变成标准ES格式
     * poutbuf指向的内存，需要调用recycle显式释放。
     * @param poutbuf
     * @param poutbuf_size
     * @param buf
     * @param buf_size
     * @param args　私有全局头,暂时用不到。
     * @return　正确返回１，其它情况返回错误码。
     *
     */
    int h264_mp4toannexb_filter(uint8_t **poutbuf, int *poutbuf_size,
                                const uint8_t *buf, int buf_size, const char *args = NULL);
    /**
     * @brief 释放由h264_mp4toannexb_filter释放的内存
     * @param p
     */
    void recycle(uint8_t** p) { av_free(*p); *p = NULL;}
private:
    void toStand(const uint8_t* buf, uint32_t size);
    int h264_extradata_to_annexb(H264BSFContext *ctx, AVCodecContext *avctx, const int padding);
    int alloc_and_copy(uint8_t **poutbuf, int *poutbuf_size,
                              const uint8_t *sps_pps, uint32_t sps_pps_size,
                              const uint8_t *in, uint32_t in_size);
    void av_free(void *ptr)    {        free(ptr);    }
    void av_log(void* /*avcl*/, int /*level*/, const char */*fmt*/, ...)    {    }
    int av_reallocp(void *ptr, size_t size);
    void *av_realloc(void *ptr, size_t size);
    void av_freep(void *arg)
    {
        void *val;

        memcpy(&val, arg, sizeof(val));
        // wzd        memcpy(arg, &(void *){ NULL }, sizeof(val));
        av_free(val);
    }
    uint16_t av_bswap16(uint16_t x)
    {
        x= (x>>8) | (x<<8);
        return x;
    }
};


#define ADTS_HEADER_SIZE 7
#define ADTS_MAX_FRAME_BYTES ((1 << 13) - 1)
#include "bbits.h"
const uint8_t ff_mpeg4audio_channels[8] = {
    0, 1, 2, 3, 4, 5, 6, 8
};
const int avpriv_mpeg4audio_sample_rates[16] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))
#define AVERROR_INVALIDDATA        FFERRTAG( 'I','N','D','A') ///< Invalid data found when processing input

#define MAX_PCE_SIZE 320 ///<Maximum size of a PCE including the 3-bit ID_PCE
class aac_asc2adts
{

    enum AudioObjectType {
        AOT_NULL,
                                   // Support?                Name
        AOT_AAC_MAIN,              ///< Y                       Main
        AOT_AAC_LC,                ///< Y                       Low Complexity
        AOT_AAC_SSR,               ///< N (code in SoC repo)    Scalable Sample Rate
        AOT_AAC_LTP,               ///< Y                       Long Term Prediction
        AOT_SBR,                   ///< Y                       Spectral Band Replication
        AOT_AAC_SCALABLE,          ///< N                       Scalable
        AOT_TWINVQ,                ///< N                       Twin Vector Quantizer
        AOT_CELP,                  ///< N                       Code Excited Linear Prediction
        AOT_HVXC,                  ///< N                       Harmonic Vector eXcitation Coding
        AOT_TTSI             = 12, ///< N                       Text-To-Speech Interface
        AOT_MAINSYNTH,             ///< N                       Main Synthesis
        AOT_WAVESYNTH,             ///< N                       Wavetable Synthesis
        AOT_MIDI,                  ///< N                       General MIDI
        AOT_SAFX,                  ///< N                       Algorithmic Synthesis and Audio Effects
        AOT_ER_AAC_LC,             ///< N                       Error Resilient Low Complexity
        AOT_ER_AAC_LTP       = 19, ///< N                       Error Resilient Long Term Prediction
        AOT_ER_AAC_SCALABLE,       ///< N                       Error Resilient Scalable
        AOT_ER_TWINVQ,             ///< N                       Error Resilient Twin Vector Quantizer
        AOT_ER_BSAC,               ///< N                       Error Resilient Bit-Sliced Arithmetic Coding
        AOT_ER_AAC_LD,             ///< N                       Error Resilient Low Delay
        AOT_ER_CELP,               ///< N                       Error Resilient Code Excited Linear Prediction
        AOT_ER_HVXC,               ///< N                       Error Resilient Harmonic Vector eXcitation Coding
        AOT_ER_HILN,               ///< N                       Error Resilient Harmonic and Individual Lines plus Noise
        AOT_ER_PARAM,              ///< N                       Error Resilient Parametric
        AOT_SSC,                   ///< N                       SinuSoidal Coding
        AOT_PS,                    ///< N                       Parametric Stereo
        AOT_SURROUND,              ///< N                       MPEG Surround
        AOT_ESCAPE,                ///< Y                       Escape Value
        AOT_L1,                    ///< Y                       Layer 1
        AOT_L2,                    ///< Y                       Layer 2
        AOT_L3,                    ///< Y                       Layer 3
        AOT_DST,                   ///< N                       Direct Stream Transfer
        AOT_ALS,                   ///< Y                       Audio LosslesS
        AOT_SLS,                   ///< N                       Scalable LosslesS
        AOT_SLS_NON_CORE,          ///< N                       Scalable LosslesS (non core)
        AOT_ER_AAC_ELD,            ///< N                       Error Resilient Enhanced Low Delay
        AOT_SMR_SIMPLE,            ///< N                       Symbolic Music Representation Simple
        AOT_SMR_MAIN,              ///< N                       Symbolic Music Representation Main
        AOT_USAC_NOSBR,            ///< N                       Unified Speech and Audio Coding (no SBR)
        AOT_SAOC,                  ///< N                       Spatial Audio Object Coding
        AOT_LD_SURROUND,           ///< N                       Low Delay MPEG Surround
        AOT_USAC,                  ///< N                       Unified Speech and Audio Coding
    };
    typedef struct MPEG4AudioConfig {
        int object_type;
        int sampling_index;
        int sample_rate;
        int chan_config;
        int sbr; ///< -1 implicit, 1 presence
        int ext_object_type;
        int ext_sampling_index;
        int ext_sample_rate;
        int ext_chan_config;
        int channels;
        int ps;  ///< -1 implicit, 1 presence
        int frame_length_short;
    } MPEG4AudioConfig;

    typedef struct ADTSContext {
        int write_adts;
        unsigned int objecttype;
        int sample_rate_index;
        int channel_conf;
        size_t pce_size;
        int apetag;
        int id3v2tag;
        uint8_t pce_data[MAX_PCE_SIZE];
    } ADTSContext;
    ADTSContext _adts_cxt;
    std::auto_ptr<uint8_t> _extra;
    uint32_t _extr_size;
public:
    aac_asc2adts(uint8_t* buf, uint32_t size)
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
            ;//,E_INVALIDARG,"MPEG-4 AOT %d is not allowed in ADTS",_adts_cxt.objecttype+1);
        if (15 == _adts_cxt.sample_rate_index)
            ;//,E_INVALIDARG,"Escape sample rate index illegal in ADTS");
        if (0 != get_bits(&gb, 1))
            ;//,E_INVALIDARG,"960/120 MDCT window is not allowed in ADTS");
        if (0 != get_bits(&gb, 1))
            ;//,E_INVALIDARG,"Scalable configurations are not allowed in ADTS");
        if (0 != get_bits(&gb, 1))
            ;//,E_INVALIDARG,"Extension flag is not allowed in ADTS");

        if (!_adts_cxt.channel_conf) {
            init_put_bits(&pb, _adts_cxt.pce_data, MAX_PCE_SIZE);

            put_bits(&pb, 3, 5); //ID_PCE
            _adts_cxt.pce_size = (avpriv_copy_pce_data(&pb, &gb) + 3) / 8;
            flush_put_bits(&pb);
        }

        _adts_cxt.write_adts = 1;
    }

size_t aac_adtstoasc_filter(uint8_t* in, size_t insize,uint8_t **buf,size_t *size)
{
    *size = (unsigned)ADTS_HEADER_SIZE + _adts_cxt.pce_size + insize;
    *buf = new uint8_t[*size];
    uint32_t  sz = adts_write_frame_header(*buf,*size);
    memcpy(*buf +sz, in,insize);
    return 0;
}
size_t adts_write_frame_header(uint8_t* buf, size_t size)
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
void recycle(uint8_t** p) {delete [] *p; *p = NULL;}
private:
 int parse_config_ALS(GetBitContext *gb, MPEG4AudioConfig *c)
{
    if (get_bits_left(gb) < 112)
        return -1;

    if (get_bits_long(gb, 32) != MKBETAG('A','L','S','\0'))
        return -1;

    // override AudioSpecificConfig channel configuration and sample rate
    // which are buggy in old ALS conformance files
    c->sample_rate = get_bits_long(gb, 32);

    // skip number of samples
    skip_bits_long(gb, 32);

    // read number of channels
    c->chan_config = 0;
    c->channels    = get_bits(gb, 16) + 1;

    return 0;
}

 int get_object_type(GetBitContext *gb)
{
    int object_type = get_bits(gb, 5);
    if (object_type == AOT_ESCAPE)
        object_type = 32 + get_bits(gb, 6);
    return object_type;
}
  int get_sample_rate(GetBitContext *gb, int *index)
 {
     *index = get_bits(gb, 4);
     return *index == 0x0f ? get_bits(gb, 24) :
         avpriv_mpeg4audio_sample_rates[*index];
 }
int avpriv_mpeg4audio_get_config(MPEG4AudioConfig *c, const uint8_t *buf,
                                 int bit_size, int sync_extension)
{
    GetBitContext gb;
    int specific_config_bitindex, ret;

    if (bit_size <= 0)
        return AVERROR_INVALIDDATA;

    ret = init_get_bits(&gb, buf, bit_size);
    if (ret < 0)
        return ret;

    c->object_type = get_object_type(&gb);
    c->sample_rate = get_sample_rate(&gb, &c->sampling_index);
    c->chan_config = get_bits(&gb, 4);
    if (c->chan_config < FF_ARRAY_ELEMS(ff_mpeg4audio_channels))
        c->channels = ff_mpeg4audio_channels[c->chan_config];
    c->sbr = -1;
    c->ps  = -1;
    if (c->object_type == AOT_SBR || (c->object_type == AOT_PS &&
        // check for W6132 Annex YYYY draft MP3onMP4
        !(show_bits(&gb, 3) & 0x03 && !(show_bits(&gb, 9) & 0x3F)))) {
        if (c->object_type == AOT_PS)
            c->ps = 1;
        c->ext_object_type = AOT_SBR;
        c->sbr = 1;
        c->ext_sample_rate = get_sample_rate(&gb, &c->ext_sampling_index);
        c->object_type = get_object_type(&gb);
        if (c->object_type == AOT_ER_BSAC)
            c->ext_chan_config = get_bits(&gb, 4);
    } else {
        c->ext_object_type = AOT_NULL;
        c->ext_sample_rate = 0;
    }
    specific_config_bitindex = get_bits_count(&gb);

    if (c->object_type == AOT_ALS) {
        skip_bits(&gb, 5);
        if (show_bits_long(&gb, 24) != MKBETAG('\0','A','L','S'))
            skip_bits_long(&gb, 24);

        specific_config_bitindex = get_bits_count(&gb);

        if (parse_config_ALS(&gb, c))
            return -1;
    }

    if (c->ext_object_type != AOT_SBR && sync_extension) {
        while (get_bits_left(&gb) > 15) {
            if (show_bits(&gb, 11) == 0x2b7) { // sync extension
                get_bits(&gb, 11);
                c->ext_object_type = get_object_type(&gb);
                if (c->ext_object_type == AOT_SBR && (c->sbr = get_bits1(&gb)) == 1) {
                    c->ext_sample_rate = get_sample_rate(&gb, &c->ext_sampling_index);
                    if (c->ext_sample_rate == c->sample_rate)
                        c->sbr = -1;
                }
                if (get_bits_left(&gb) > 11 && get_bits(&gb, 11) == 0x548)
                    c->ps = get_bits1(&gb);
                break;
            } else
                get_bits1(&gb); // skip 1 bit
        }
    }

    //PS requires SBR
    if (!c->sbr)
        c->ps = 0;
    //Limit implicit PS to the HE-AACv2 Profile
    if ((c->ps == -1 && c->object_type != AOT_AAC_LC) || c->channels & ~0x01)
        c->ps = 0;

    return specific_config_bitindex;
}
int avpriv_copy_pce_data(PutBitContext *pb, GetBitContext *gb)
{
    int five_bit_ch, four_bit_ch, comment_size, bits;
    int offset = put_bits_count(pb);

    copy_bits(pb, gb, 10);                  //Tag, Object Type, Frequency
    five_bit_ch  = copy_bits(pb, gb, 4);    //Front
    five_bit_ch += copy_bits(pb, gb, 4);    //Side
    five_bit_ch += copy_bits(pb, gb, 4);    //Back
    four_bit_ch  = copy_bits(pb, gb, 2);    //LFE
    four_bit_ch += copy_bits(pb, gb, 3);    //Data
    five_bit_ch += copy_bits(pb, gb, 4);    //Coupling
    if (copy_bits(pb, gb, 1))               //Mono Mixdown
        copy_bits(pb, gb, 4);
    if (copy_bits(pb, gb, 1))               //Stereo Mixdown
        copy_bits(pb, gb, 4);
    if (copy_bits(pb, gb, 1))               //Matrix Mixdown
        copy_bits(pb, gb, 3);
    for (bits = five_bit_ch*5+four_bit_ch*4; bits > 16; bits -= 16)
        copy_bits(pb, gb, 16);
    if (bits)
        copy_bits(pb, gb, bits);
    avpriv_align_put_bits(pb);
    align_get_bits(gb);
    comment_size = copy_bits(pb, gb, 8);
    for (; comment_size > 0; comment_size--)
        copy_bits(pb, gb, 8);

    return put_bits_count(pb) - offset;
}
const uint8_t *align_get_bits(GetBitContext *s)
{
    int n = -get_bits_count(s) & 7;
    if (n)
        skip_bits(s, n);
    return s->buffer + (s->index >> 3);
}
void avpriv_align_put_bits(PutBitContext *s)
{
    put_bits(s, s->bit_left & 7, 0);
}
unsigned int copy_bits(PutBitContext *pb,
                                               GetBitContext *gb,
                                               int bits)
{
    unsigned int el = get_bits(gb, bits);
    put_bits(pb, bits, el);
    return el;
}
};


#endif // BITSTREAMFILTER_H

