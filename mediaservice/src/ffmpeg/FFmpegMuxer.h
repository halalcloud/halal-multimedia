#ifndef FFMPEGMUXER_H
#define FFMPEGMUXER_H
#include "stdafx.h"
#include <list>
#include <vector>
#include <memory>

class CFFmpegMuxer : public IFilter ,public ILoad
{
    typedef list< dom_ptr<IMediaFrame> > FrameSet;
    typedef FrameSet::iterator FrameIt;
    class CStream
    {
        friend class CFFmpegMuxer;
    public:
        CStream(CFFmpegMuxer* pMuxer);
        virtual ~CStream();
        HRESULT Open();
        HRESULT Push(IMediaFrame* pFrame);
        HRESULT Peek(IMediaFrame** ppFrame);
        bool Pop();
        bool IsEnd();
        HRESULT Convert(AVPacket& pkt,IMediaFrame* pFrame);
        HRESULT Close();
        bool IsConnect();
    protected:
        CFFmpegMuxer* m_pMuxer;
        AVStream* m_pStream;
        dom_ptr<IInputPin> m_spPin;
        dom_ptr<IMediaType> m_spMT;
        IMediaType* m_pMT;
        AVBitStreamFilterContext* m_ctxBSF;
        bool m_isGlobalHeader;
        bool m_preprocess;
        AVPacket m_pktOut;
        FrameSet m_frames;
        bool m_isEnd;
    };
    friend class CStream;
    typedef vector<CStream*> StreamSet;
    typedef StreamSet::iterator StreamIt;
public:
    DOM_DECLARE(CFFmpegMuxer)
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
    //CFFmpegMuxer
    MediaSubType GetFormatSubType(MediaMajorType major);
    bool QueryFormatSubType(MediaSubType sub);
    HRESULT Open();
    HRESULT Close();
    HRESULT Write();
    HRESULT WriteFrame(CStream* pStream,IMediaFrame* pFrame);
    static int timeout_callback(void *pParam);
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    StreamSet m_streams;
    AVFormatContext* m_ctxFormat;
    IFilter::Status m_status;
    bool m_isOpen;
    void* m_pTag;
    bool m_isFirst;
    bool m_isLive;
    bool m_isGlobalHeader;
    bool m_isImage;
    bool m_isImageWrite;
    bool m_isSegment;
    int64_t m_pos;
    int64_t m_dts;
    AVPacket m_pkt;
    AVRational m_base;
    time_expend m_te;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
};

#endif // FFMPEGMUXER_H
