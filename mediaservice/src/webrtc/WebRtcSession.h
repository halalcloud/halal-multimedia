#ifndef WEBRTCSESSION_H
#define WEBRTCSESSION_H
#include "stdafx.h"

class CWebRtcSession : public IFilter, public ILoad, public ICallback
{
public:
    DOM_DECLARE(CWebRtcSession)
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
    //CWebRtcSession
    HRESULT Init();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    FilterType m_type;
    IFilter::Status m_status;
    string m_name;
    void* m_pTag;
    dom_ptr<IMediaFrameAllocate> m_spAllocate;
    vector< dom_ptr<IInputPin> > m_pinsIn;
    vector< dom_ptr<IOutputPin> > m_pinsOut;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
};

#endif // WEBRTCSESSION_H
