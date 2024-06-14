#ifndef FFMPEGDEMUXER_H
#define FFMPEGDEMUXER_H

#include "stdafx.h"
#define MAX_PCE_SIZE 320
class CFFmpegDemuxer : public IFilter , public IDemuxer
{
    class CStream
    {
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
        friend class CFFmpegDemuxer;
    public:
        CStream();
        virtual ~CStream();
        HRESULT SetStream(CFFmpegDemuxer* pDemuxer,size_t index,AVStream* pStream);
        bool IsOutput();
        HRESULT Open();
        HRESULT Process(AVPacket& pkt);
        void Close();
    protected:
        HRESULT ProcessOut(AVPacket& pkt,IMediaFrame* pFrame);
        HRESULT adts_decode_extradata(const uint8_t *buf, size_t size);
        size_t adts_write_frame_header(uint8_t *buf,size_t size);
        HRESULT adts_write_packet(AVPacket& pkt,IMediaFrame* pFrame);
        HRESULT Output(IMediaFrame* pFrame);
    protected:
        CFFmpegDemuxer* m_pDemuxer;
        AVStream* m_pStream;
        dom_ptr<IMediaType> m_spMT;
        dom_ptr<IOutputPin> m_spPin;
        AVBitStreamFilterContext* m_ctxBSF;
        ADTSContext* m_adts;
        IMediaType* m_pMT;
        int64_t m_duration;
        int64_t m_input_dts;
        int64_t m_delta;
        uint32_t m_delta_count;
        bool m_isGlobalHeader;
        int64_t m_start;
        int64_t m_length;
        bool m_eof;
    };
    friend class CStream;
public:
    DOM_DECLARE(CFFmpegDemuxer)
    //IFilter
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(uint32_t) GetInputPinCount();
    STDMETHODIMP_(IInputPin*) GetInputPin(uint32_t index);
    STDMETHODIMP_(uint32_t) GetOutputPinCount();
    STDMETHODIMP_(IOutputPin*) GetOutputPin(uint32_t index);
    STDMETHODIMP Open();
    STDMETHODIMP Close();
    STDMETHODIMP SetTag(void* pTag);
    STDMETHODIMP_(void*) GetTag();
    STDMETHODIMP_(double) GetExpend();
    STDMETHODIMP OnGetMediaType(IInputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnGetMediaType(IOutputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnSetMediaType(IInputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnSetMediaType(IOutputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    STDMETHODIMP OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame);
    //IDemuxer
    STDMETHODIMP Load(const char* pUrl);
    STDMETHODIMP_(IOutputPin*) CreatePin(IMediaType* pMT);
    STDMETHODIMP_(void) SetStartTime(const int64_t& time = MEDIA_FRAME_NONE_TIMESTAMP);
    STDMETHODIMP_(int64_t) GetTime();
    STDMETHODIMP Process();
    STDMETHODIMP_(void) Clear();
    STDMETHODIMP_(bool) IsEOF();
    STDMETHODIMP_(void) NewSegment();
    //CFFmpegDemuxer
    void SetDelta(const int64_t& delta);
    void SetDelta(int64_t& o,const int64_t& n,const int64_t& offset);
    static int timeout_callback(void *pParam);
    static HRESULT UrlSupportQuery(const char* pProtocol,const char* pFormat);
protected:
    string m_name;
    IFilterGraphEvent* m_pFG;
    AVFormatContext* m_ctxFormat;
    CStream* m_streams;
    unsigned int m_count;
    bool m_isEOF;
    bool m_isOpen;
    void* m_pTag;
    int64_t m_time;
    int64_t m_delta;
    uint32_t m_delta_count;
    int64_t m_begin;
    bool m_isGlobalHeader;
    dom_ptr<Interface> m_spProfile;
    time_expend m_te;
    int64_t m_timeout;
};

#endif // FFMPEGDEMUXER_H
