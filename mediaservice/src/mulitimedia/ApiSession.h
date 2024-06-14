#ifndef APISESSION_H
#define APISESSION_H

#include "stdafx.h"


class CMediaService;
class CApiSession : public IFilter, public ILoad, public ICallback
{
    friend class CMediaService;
public:
    DOM_DECLARE(CApiSession)
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
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    HRESULT SetType(FilterType type);
    //static HRESULT CreateBySession(uint32_t type,CMediaService* pService,IFilter* pFilter);
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    dom_ptr<IProfile> m_spProfile;
    FilterType m_type;
    uint32_t m_flag;
    IFilter::Status m_status;
    void* m_pTag;
    dom_ptr<IInputPin> m_pinIn;
    dom_ptr<IOutputPin> m_pinOut;
    dom_ptr<IEventPoint> m_ep;
    string m_stream;
    string m_method;
};

#endif // APISESSION_H
