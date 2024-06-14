#include "PublishRender.h"

CPublishRender::CPublishRender()
    :m_status(IFilter::S_Stop)
    ,m_isOpen(false)
    ,m_pTag(NULL)
    ,m_gop(0)

{
    //ctor
}

bool CPublishRender::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(NULL != (m_sessions = SET(SessionSet)::Create(NULL,false,(ICallback*)this)),false);
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    return true;
}

bool CPublishRender::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Notify(S_Stop);
        if(m_pinOut != NULL)
            m_pinOut->Disconnect();
        LOG(0,"%s[%p] name:%s release",Class().name,(IFilter*)this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CPublishRender)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT(IEventCallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CPublishRender::GetType()
{
    return FT_Render;
}

STDMETHODIMP CPublishRender::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CPublishRender::GetName()
{
    return true == m_name.empty() ? NULL : m_name.c_str();
}

STDMETHODIMP_(uint32_t) CPublishRender::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CPublishRender::GetInputPinCount()
{
    return m_pinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CPublishRender::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn : NULL;
}

STDMETHODIMP_(uint32_t) CPublishRender::GetOutputPinCount()
{
    return m_pinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CPublishRender::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CPublishRender::CreateInputPin(IMediaType* pMT)
{
    JCHK(NULL != pMT,NULL);
    JCHK(NULL == m_pinIn,NULL);

    if(MMT_DATA != pMT->GetMajor())
        return NULL;

    dom_ptr<IInputPin> spPinIn;
    dom_ptr<IOutputPin> spPinOut;

    JCHK(spPinIn.Create(CLSID_CInputPin,(IFilter*)this),NULL);
    JCHK(spPinOut.Create(CLSID_COutputPin,(IFilter*)this),NULL);

    if(MST_NONE == pMT->GetSub() && false == m_url.m_format.empty())
    {
        pMT->SetSub(m_url.m_format.c_str());
    }

    JCHK(S_OK == spPinIn->SetMediaType(pMT),NULL);
    JCHK(S_OK == spPinOut->SetMediaType(pMT),NULL);

    m_pinIn = spPinIn;
    m_pinOut = spPinOut;

    return spPinIn.p;
}

STDMETHODIMP_(IOutputPin*) CPublishRender::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CPublishRender::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < IFilter::S_NB)
    {
        if(cmd != m_status)
        {
            if(S_Stop == m_status)
            {
                JIF(m_ep->Notify(ET_Publish_Add,0,m_pinOut.p));
            }
            else if(S_Stop == cmd)
            {
                JIF(m_ep->Notify(ET_Publish_Del,0,m_pinOut.p));
            }
            if(S_Play == cmd)
                m_pinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CPublishRender::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CPublishRender::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CPublishRender::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CPublishRender::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CPublishRender::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    if(NULL != pMT)
    {
        if((MMT_DATA != pMT->GetMajor() || MST_NONE == pMT->GetSub()))
            return E_INVALIDARG;

        m_name = m_url.GetStreamID(pMT->GetSubName());

        dom_ptr<IProfile> spProfile;
        JCHK(spProfile.QueryFrom(pMT),E_FAIL);

        JCHK(false == m_name.empty(),E_FAIL);

        JCHK(NULL != spProfile->Write("stream_id",m_name.c_str(),m_name.size()+1),E_FAIL);
    }
    else
        m_pinOut->Disconnect();
    return m_pinOut->SetMediaType(pMT);
}

STDMETHODIMP CPublishRender::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        if(pMT != m_pinIn->GetMediaType())
            return E_FAIL;

        dom_ptr<IEventPoint> spEP;
        JCHK(spEP.QueryFrom(pPinIn->GetFilter()),E_FAIL);
        JIF(spEP->NotifySet((IFilter*)this));
    }
    return hr;
}

STDMETHODIMP CPublishRender::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    JCHK(m_pinIn == pPin,E_INVALIDARG);

    HRESULT hr;
    {
        //printf("CPublishRender::OnWriteFrame dts:%ld flag:%u\n",pFrame->info.dts,pFrame->info.flag);
        CLocker locker(&m_locker);
        if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_SEGMENT))
            pFrame->info.flag |= MEDIA_FRAME_FLAG_EOF;
        JIF(FrameBufferInput(m_frames,pFrame,pPin->GetBufLen(),m_gop));
    }
    m_pinOut->Write(pFrame);

    return hr;
}

STDMETHODIMP CPublishRender::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    if(NULL == pUrl)
        pUrl = m_name.c_str();

    return m_url.Set(pUrl);
}

STDMETHODIMP CPublishRender::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(ET_EOF == type)
    {
        dom_ptr<IFilter> spFilter;
        if(true == spFilter.QueryFrom(source))
        {
            if(FT_Render == spFilter->GetType())
            {
                spFilter->GetInputPin(0)->Disconnect();
                LOG(0,"%s[%p] %s[%p]:[%s] finish return:%d",Class().name,this,spFilter->Class().name,source,spFilter->GetName(),param1);
                return S_OK;
            }
        }
    }

    else if(ET_Filter_Buffer == type)
    {
        HRESULT hr = S_OK;
        IInputPin* pPinIn;
        JCHK(pPinIn = (IInputPin*)param2,E_INVALIDARG);

        char* stream = (char*)param3;

        if(NULL != pPinIn->GetTag())
            return hr;

        int64_t pos = MEDIA_FRAME_NONE_TIMESTAMP;

        if(stream != NULL)
        {
            string name = m_url.GetStreamName();
            if(name != stream)
            {
                char* pEnd = stream;
                pos = strtoll(stream,&pEnd,10);
                if(pEnd <= stream)
                    pos = MEDIA_FRAME_NONE_TIMESTAMP;
            }
        }
        {
            CLocker locker(m_locker);
            HRESULT hr = S_OK;
            for(FrameIt it = m_frames.begin() ; it != m_frames.end() ; ++it)
            {
                dom_ptr<IMediaFrame>& spFrame = *it;
                if(MEDIA_FRAME_NONE_TIMESTAMP == pos)
                {
                    JIF(pPinIn->Write(spFrame));
                    if(0 != (spFrame->info.flag & MEDIA_FRAME_FLAG_EOF))
                        return hr;
                }
                else
                {
                    if(pos == spFrame->info.dts)
                    {
                        JIF(pPinIn->Write(spFrame));
                        return hr;
                    }
                    else
                        hr = E_FAIL;
                }
            }
            if(S_OK == hr)
            {
                pPinIn->SetTag(this);
            }
            else
            {
                LOG(0,"publish:%s buffer not find %ld frame",m_name.c_str(),pos);
            }
        }
        return hr;
    }
    else if(ET_Filter_Render == type)
    {
        return S_OK;
    }
    return m_ep->Notify(type,param1,param2,param3);
}

STDMETHODIMP_(void) CPublishRender::Set(Interface* obj,bool erase)
{
}

HRESULT CPublishRender::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(STR_CMP(PROTOCOL_PUBLISH_NAME,protocol))
        return 1;
    else
        return E_INVALIDARG;
}
