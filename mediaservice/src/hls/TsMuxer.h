#ifndef TSMUXER_H
#define TSMUXER_H
#include <vector>
#include "stdafx.h"
#include <memory>
#include <chrono>
#include <list>
#include <sstream>
#include <string>
#include <strstream>


#include "bbits.h"
namespace wzd
{
#include "tsmuxer/TsMuxer.h"
#include "tsmuxer/libmpeg_sink.h"
#include "auto_buffer.h"


#define ADTS_MAX_FRAME_BYTES ((1 << 13) - 1)

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
    static const int  ADTS_HEADER_SIZE = 7;
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
    auto_buffer<uint8_t> _extra;
    uint32_t _extr_size;
public:
    /**
     * @brief aac_asc2adts
     * @param buf
     * @param size
     * @throw std::exception
     */
    aac_asc2adts(uint8_t* buf, uint32_t size);
    /**
     * @brief aac_asc2adts_filter
     * @param in
     * @param insize
     * @param buf
     * @param size
     * @return
     */
    size_t aac_asc2adts_filter(uint8_t* in, size_t insize,uint8_t **buf,size_t *size);
    /**
     * @brief recycle
     * @param p
     */
    void recycle(uint8_t** p) {delete [] *p; *p = NULL;}
private:
    size_t adts_write_frame_header(uint8_t* buf, size_t size);
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



class h264_mp4toannexb
{
    typedef struct _StandardContext
    {
        auto_buffer<uint8_t> _data; // for non-standard stream
        uint32_t _data_size;
        uint32_t _used;
    }StandardContext;
    auto_buffer<uint8_t>  _extra_data;
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
     * @throw runtime_error
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
        struct buf_deleter {  // a deleter class with state
      buf_deleter(h264_mp4toannexb*& convert):pConvert(convert){}
      void operator()(uint8_t* p) {
          pConvert->recycle(&p);
      }
      h264_mp4toannexb* pConvert;
    };
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

using namespace std;
class M3u8Generator
{
    struct SEG {
        string _f;
        double _dur;
        SEG(char* f, double dur)
            :_f(f)
            ,_dur(dur)

        {

        }
    };
uint64_t _seq;
    uint16_t _segCnt;
    std::list<SEG> _segs; // file loc

    string gen()
    {
        uint64_t dur = 0;
        ostringstream m3u8;

        for (auto s : _segs)
        {
            m3u8 << "#EXTINF:" << s._dur <<"," << "no_desc" <<endl;
             m3u8 << s._f << endl;
             //dur += s._dur;
             if (s._dur +0.5 > dur)
                dur = s._dur+0.5;
        }
        ostringstream m3u8H;
        m3u8H <<  "#EXTM3U";
        m3u8H << endl;
                m3u8H << "#EXT-X-VERSION:3"<<endl;
        m3u8H << "#EXT-X-TARGETDURATION:";
        m3u8H << dur << endl;
        m3u8H << "#EXT-X-MEDIA-SEQUENCE:"
        << _seq << endl;

        m3u8H << "#EXT-X-ALLOW_CACHE:NO"<<endl;
        m3u8H << m3u8.str().c_str() << endl;
        //m3u8H << "#EXT-X-ENDLIST";
        return m3u8H.str();
    }
public:
    M3u8Generator(uint16_t segCnt)
        :_segCnt(segCnt)
                    , _seq(0)
    {

    }

    ~M3u8Generator()
    {

    }
    string append(char* f, double dur, uint32_t seq)
    {
        if (_segCnt == _segs.size())
            _segs.pop_front();
        SEG s(f, dur);
        _segs.push_back(s);

            _seq = seq;

        return gen();
    }

};
}

class CTsMuxer : public IFilter, public wzd::muxer_sink
{
    typedef vector< dom_ptr<IInputPin> > InputSet;
    typedef InputSet::iterator InputIt;
public:
    DOM_DECLARE(CTsMuxer)
    //IFilter
    STDMETHODIMP_(FilterType) GetType();
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(uint32_t) GetInputPinCount();
    STDMETHODIMP_(IInputPin*) GetInputPin(uint32_t index);
    STDMETHODIMP_(uint32_t) GetOutputPinCount();
    STDMETHODIMP_(IOutputPin*) GetOutputPin(uint32_t index);
    STDMETHODIMP_(IInputPin*) CreateInputPin(IMediaType* pMT);
    STDMETHODIMP_(IOutputPin*) CreateOutputPin(IMediaType* pMT);
    STDMETHODIMP Notify(uint32_t cmd);
    STDMETHODIMP_(uint32_t) GetStatus();
    STDMETHODIMP_(void) SetTag(void* pTag);
    STDMETHODIMP_(void*) GetTag();
    STDMETHODIMP_(double) GetExpend();
    STDMETHODIMP OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT);
    STDMETHODIMP OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    //CTsMuxer
    HRESULT Open();
    HRESULT Close();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);

    HRESULT write(unsigned char *p, unsigned int size);

protected:
    string m_name;
    InputSet m_inputs;
    dom_ptr<IOutputPin> m_pinOut;
    dom_ptr<IProfile> m_slices;
    vector<uint32_t> m_indexs;
    IFilter::Status m_status;
    bool m_isOpen;
    bool m_isFirst;
    bool m_is_live;
    void* m_pTag;
    uint32_t m_index;
    uint32_t m_master;
    dom_ptr<IProfile> m_spProfile;
    dom_ptr<IEventPoint> m_ep;
    CUrl m_url;
    CLocker m_locker;
private:
    std::shared_ptr<wzd::CTsMuxer> _mux;
    bool _hasA;
    bool _hasV;
    dom_ptr<IMediaFrame> _frm_flags;

    dom_ptr<IMediaFrame> _buf_frm;
    bool _genM3u8;
    std::chrono::seconds _dur;
    dom_ptr<IStream> m_spOutput;
dom_ptr<IMediaFrame> _ff;//first frame
    bool _flash;
    std::unique_ptr<wzd::M3u8Generator> _m3;
    enum FMT
    {
    ASC
    ,ADTS
    ,MP4
    ,ANNEXB

    ,
    };
    FMT _fmtA;
    FMT _fmtV;
    wzd::aac_asc2adts* _asc2adts;
    wzd::h264_mp4toannexb* _mp4To;
    int _segCnt;
};

#endif // TSMUXER_H
