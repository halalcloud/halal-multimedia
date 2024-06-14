#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H
#include "stdafx.h"
#include <alsa/asoundlib.h>
#include <memory>
#include <time_expend.h>
const uint32_t VIDEO_BUFFER_COUNT = 4;
struct buffer
{
    uint8_t* data;
    uint32_t size;
};
//#define WRITE_PCM
class CCapture : public IFilter, public IDemuxer
{
    class CStream
    {
        friend class CCapture;
    public:
        CStream(CCapture* pCapture);
        virtual ~CStream();
        HRESULT Set(const char* pName,IMediaType* pMT,size_t index);
        HRESULT Open();
        HRESULT Process();
        HRESULT Output(IMediaFrame* pFrame);
        HRESULT Close();
    protected:
        CCapture* m_pCapture;
        dom_ptr<IMediaFrame> m_spFrame;
        dom_ptr<IOutputPin> m_spPin;
        dom_ptr<IMediaType> m_spMT;
        IMediaType* m_pMT;
        uint32_t m_index;
        VideoMediaType m_pix_fmt;
        int m_width;
        int m_height;
        int m_ratioX;
        int m_ratioY;
        int64_t m_duration;
        int m_stride;
        buffer* m_bufs;
        int64_t m_time;
        int64_t m_delta;
        string m_name;
        int m_fd;
    private:
        snd_pcm_t * _capture_handle;
        uint32_t _bufLen ;
        unsigned int _frequency;
        AudioMediaType _fmt;
        int _ch;
        int _fs;
    #ifdef WRITE_PCM
    FILE* _fp;
    #endif // WRITE_PCM
    };
    friend class CStream;
    typedef vector<CStream*> StreamSet;
    typedef StreamSet::iterator StreamIt;
public:
    DOM_DECLARE(CCapture)
    //IFilter
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
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
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(IOutputPin*) CreatePin(IMediaType* pMT);
    STDMETHODIMP_(void) SetStartTime(const int64_t& time = MEDIA_FRAME_NONE_TIMESTAMP);
    STDMETHODIMP_(int64_t) GetTime();
    STDMETHODIMP Process();
    STDMETHODIMP_(void) Clear();
    STDMETHODIMP_(bool) IsEOF();
    STDMETHODIMP_(void) NewSegment();
    //CSdlRender
    static HRESULT UrlSupportQuery(const char* pProtocol,const char* pFormat);
protected:
    string m_name;
    IFilterGraphEvent* m_pFG;
    StreamSet m_streams;
    dom_ptr<Interface> m_spProfile;
    void* m_tag;
    bool m_isOpen;
    bool m_isEOF;
    int64_t m_begin;
    int64_t m_start;
    int64_t m_time;
    int64_t m_delta;
    uint32_t m_index[MMT_NB];
    time_expend m_te;
    CStream* m_pMaster;
};

#endif // V4L2CAPTURE_H
