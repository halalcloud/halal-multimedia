#ifndef PUBLISHRENDER_H
#define PUBLISHRENDER_H
#include "stdafx.h"

class CPublishRender : public IFilter , public ILoad , public IEventCallback
{
    typedef list< dom_ptr<IFilter> > SessionSet;
    typedef SessionSet::iterator SessionIt;
public:
    DOM_DECLARE(CPublishRender)
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
    //ISerialize
    STDMETHODIMP Load(IStream* pStream,uint8_t flag);
    STDMETHODIMP Save(IStream* pStream,uint8_t flag);
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    STDMETHODIMP_(void) Set(Interface* obj,bool erase);
    //CPublishRender
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    IFilter::Status m_status;
    bool m_isOpen;
    void* m_pTag;
    string m_name;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
    dom_ptr<IInputPin> m_pinIn;
    dom_ptr<IOutputPin> m_pinOut;
    CUrl m_url;
    dom_ptr<SET(SessionSet)> m_sessions;
    FrameSet m_frames;
    CLocker m_locker;
    uint32_t m_gop;
};

#endif // PUBLISHRENDER_H
