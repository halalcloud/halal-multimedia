#ifndef FFMPEGMUXER_H
#define FFMPEGMUXER_H
#include "stdafx.h"
#include <list>
#include <vector>
#include <memory>

class CFFmpegMuxer : public IFilter , public IMuxer
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
    //IMuxer
    STDMETHODIMP Load(const char* pUrl);
    STDMETHODIMP_(IInputPin*) CreatePin(IMediaType* pMT);
    STDMETHODIMP_(void) Clear();
    //CFFmpegMuxer
    MediaSubType GetFormatSubType(MediaMajorType major);
    bool QueryFormatSubType(MediaSubType sub);
    HRESULT Write();
    HRESULT WriteFrame(CStream* pStream,IMediaFrame* pFrame);
    static int timeout_callback(void *pParam);
    static HRESULT UrlSupportQuery(const char* pProtocol,const char* pFormat);
protected:
    string m_name;
    IFilterGraphEvent* m_pFG;
    StreamSet m_streams;
    AVFormatContext* m_ctxFormat;
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
    dom_ptr<Interface> m_spProfile;
    time_expend m_te;
};

#endif // FFMPEGMUXER_H
