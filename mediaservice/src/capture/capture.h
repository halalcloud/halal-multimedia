#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H
#include "stdafx.h"
#include <alsa/asoundlib.h>
#include <memory>
#include <time_expend.h>
#include <Locker.h>
const uint32_t VIDEO_BUFFER_COUNT = 4;
struct buffer
{
    uint8_t* data;
    uint32_t size;
};
//#define WRITE_PCM
class CCapture : public IFilter, public ILoad, public ICallback
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
        dom_ptr<IEpollPoint> m_epoll;
        ILocker* m_locker;
        CCapture* m_pCapture;
        dom_ptr<IMediaFrame> m_spFrame;
        dom_ptr<IOutputPin> m_spPin;
        uint32_t m_index;
        VideoMediaType m_pix_fmt;
        int m_width;
        int m_height;
        int m_ratioX;
        int m_ratioY;
        int64_t m_duration;
        int m_stride;
        buffer* m_bufs;
        int64_t m_start;
        int64_t m_time;
        string m_name;
        vector<int> m_fds;
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
    STDMETHODIMP OnGetMediaType(IInputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnGetMediaType(IOutputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT);
    STDMETHODIMP OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    //ILoad
    STDMETHODIMP Load(const char* pUrl,uint32_t mode,uint8_t flag = 0);
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    STDMETHODIMP_(bool) GetEventEnable();
    //CSdlRender
    HRESULT Open();
    HRESULT Close();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    void* m_pTag;
    StreamSet m_streams;
    IFilter::Status m_status;
    bool m_isOpen;
    bool m_isEOF;
    int64_t m_begin;
    int64_t m_start;
    int64_t m_time;
    uint32_t m_index[MMT_NB];
    time_expend m_te;
    CStream* m_pMaster;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
};

#endif // V4L2CAPTURE_H
