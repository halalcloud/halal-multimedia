#include "WebRtcSession.h"

CWebRtcSession::CWebRtcSession()
:m_type(FT_None)
,m_status(S_Stop)
,m_pTag(NULL)
{
    //ctor
}

bool CWebRtcSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    JCHK(m_spAllocate.Create(CLSID_CMediaFrameAllocate),false);
    return true;
}

bool CWebRtcSession::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Notify(S_Stop);
        LOG(0,"%s[%p] name:%s release",Class().name,(IFilter*)this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CWebRtcSession)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CWebRtcSession::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    HRESULT hr = S_OK;
    CUrl url;
    url.Set(NULL == pUrl ? m_name.c_str() : pUrl);
    m_type = (FilterType)mode;
    return hr;
}

STDMETHODIMP_(FilterType) CWebRtcSession::GetType()
{
    return m_type;
}

STDMETHODIMP CWebRtcSession::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CWebRtcSession::GetName()
{
    return true == m_name.empty() ? NULL : m_name.c_str();
}

STDMETHODIMP_(uint32_t) CWebRtcSession::GetFlag()
{
    return IFilter::FLAG_LIVE;
}

STDMETHODIMP_(uint32_t) CWebRtcSession::GetInputPinCount()
{
    return m_pinsIn.size();
}

STDMETHODIMP_(IInputPin*) CWebRtcSession::GetInputPin(uint32_t index)
{
    return m_pinsIn.size() > index ? m_pinsIn[index] : NULL;
}

STDMETHODIMP_(uint32_t) CWebRtcSession::GetOutputPinCount()
{
    return m_pinsOut.size();
}

STDMETHODIMP_(IOutputPin*) CWebRtcSession::GetOutputPin(uint32_t index)
{
    return m_pinsOut.size() > index ? m_pinsOut[index] : NULL;
}

STDMETHODIMP_(IInputPin*) CWebRtcSession::CreateInputPin(IMediaType* pMT)
{
    dom_ptr<IInputPin> spPin;
    JCHK(spPin.Create(CLSID_CInputPin,(IFilter*)this),NULL);
    spPin->SetMediaType(pMT);
    m_pinsIn.push_back(spPin);
    return spPin.p;
}

STDMETHODIMP_(IOutputPin*) CWebRtcSession::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CWebRtcSession::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(FT_Render == m_type)
            {
                if(S_Stop == m_status)
                {
                    //open
                }
                else if(S_Stop == cmd)
                {
                    //close;
                }

                if(S_Play == cmd)
                {
                    //play
                }
                else if(S_Pause == cmd)
                {
                    //pause
                }
            }
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    else
    {
        if(IFilter::C_Profile == cmd)
        {
            JIF(Init());
        }
        else if(IFilter::C_Accept == cmd)
        {
            //JIF(m_stream.Play());
        }
        else if(IFilter::C_Enable == cmd)
        {
        }
        else if(IFilter::C_Disable == cmd)
        {
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CWebRtcSession::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CWebRtcSession::SetTag(void* tag)
{
    m_pTag = tag;
}

STDMETHODIMP_(void*) CWebRtcSession::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CWebRtcSession::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CWebRtcSession::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    return S_OK;
}

STDMETHODIMP CWebRtcSession::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return S_OK;
}

STDMETHODIMP CWebRtcSession::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return S_OK;
}

//ICallback
STDMETHODIMP CWebRtcSession::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    return m_ep->Notify(type,param1,param2,param3);
}

HRESULT CWebRtcSession::Init()
{
//    IProfile::val* pVal;
//    bool is_catch = false;
//
//    m_spProfile->Read("catch",is_catch);
//    if(NULL != (pVal = m_spProfile->Read("timeout")))
//    {
//        if(STR_CMP(pVal->type,typeid(char*).name()) || STR_CMP(pVal->type,typeid(const char*).name()))
//        {
//            char* pUnit = NULL;
//            double val = strtod((const char*)pVal->value, &pUnit);
//            if(0.0 < val)
//            {
//                if(*pUnit == 'h' || *pUnit == 'H')
//                    m_timeout = int64_t(val * 60 * 60 * 1000);
//                else if(*pUnit == 'm' || *pUnit == 'M')
//                    m_timeout = int64_t(val * 60 * 1000);
//                else if(*pUnit == 's' || *pUnit == 'S')
//                    m_timeout = int64_t(val * 1000);
//                else
//                    m_timeout = int64_t(val);
//            }
//        }
//    }
    return S_OK;
}

HRESULT CWebRtcSession::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(STR_CMP(protocol,WEBRTC_PROTOCOL_NAME) ||
            (NULL == protocol && STR_CMP(format,WEBRTC_PROTOCOL_NAME)))
        return 1;
    return E_INVALIDARG;
}
