#ifndef FFMPEGDEMUXER_H
#define FFMPEGDEMUXER_H

#include "stdafx.h"
#define MAX_PCE_SIZE 320
class CFFmpegDemuxer : public IFilter , public ILoad , public ISource , public ICallback
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
    //ILoad
    STDMETHODIMP Load(const char* pUrl,uint32_t mode,uint8_t flag = 0);
    //IDemuxer
    STDMETHODIMP_(void) SetStartTime(const int64_t& time = MEDIA_FRAME_NONE_TIMESTAMP);
    STDMETHODIMP_(int64_t) GetTime();
    STDMETHODIMP Process();
    STDMETHODIMP_(void) Clear();
    STDMETHODIMP_(bool) IsEOF();
    STDMETHODIMP_(void) NewSegment();
    //ICallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    //CFFmpegDemuxer
    HRESULT Open();
    HRESULT Close();
    void SetDelta(const int64_t& delta);
    void SetDelta(int64_t& o,const int64_t& n,const int64_t& offset);
    static int timeout_callback(void *pParam);
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    int m_fd;
    void* m_event;
    uint64_t m_msg_event;
    AVFormatContext* m_ctxFormat;
    CStream* m_streams;
    unsigned int m_count;
    bool m_isEOF;
    IFilter::Status m_status;
    bool m_isOpen;
    void* m_pTag;
    int64_t m_time;
    int64_t m_delta;
    uint32_t m_delta_count;
    int64_t m_begin;
    bool m_isGlobalHeader;
    time_expend m_te;
    int64_t m_timeout;
    ILocker* m_locker;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
    dom_ptr<IEpollPoint> m_epoll;
};

#endif // FFMPEGDEMUXER_H
