#include "ApiSession.h"
#include "MediaService.h"

CApiSession::CApiSession()
:m_type(FT_None)
,m_flag(0)
,m_status(S_Stop)
,m_pTag(NULL)
{
}

bool CApiSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    return true;
}

bool CApiSession::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Notify(S_Stop);
        LOG(0,"%s[%p] name:%s release",Class().name,(IFilter*)this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CApiSession)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CApiSession::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    HRESULT hr;
    CUrl url;
    JIF(url.SetStreamID(NULL,NULL == pUrl ? m_stream.c_str() : pUrl));
    JIF(SetType((FilterType)mode));
    m_stream = url.m_host;
    m_stream += url.m_path;
    if('/' == m_stream.back())
        m_stream.erase(m_stream.size()-1);
    m_method = url.m_file;
    m_type = (FilterType)mode;
    return hr;
}

STDMETHODIMP_(FilterType) CApiSession::GetType()
{
    return m_type;
}

STDMETHODIMP CApiSession::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_stream = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CApiSession::GetName()
{
    return m_stream.empty() ? NULL : m_stream.c_str();
}

STDMETHODIMP_(uint32_t) CApiSession::GetFlag()
{
    return m_flag;
}

STDMETHODIMP_(uint32_t) CApiSession::GetInputPinCount()
{
    return m_pinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CApiSession::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn.p : NULL;
}

STDMETHODIMP_(uint32_t) CApiSession::GetOutputPinCount()
{
    return m_pinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CApiSession::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut.p : NULL;
}

STDMETHODIMP_(IInputPin*) CApiSession::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CApiSession::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CApiSession::Notify(uint32_t cmd)
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
                }
                else if(S_Stop == cmd)
                {
                }
            }
            else if(FT_Source == m_type)
            {
                if(S_Stop == m_status)
                {
                }
                else if(S_Stop == cmd)
                {
                }
            }
            m_status = (Status)cmd;
        }
    }
    else
    {
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CApiSession::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CApiSession::SetTag(void* tag)
{
    m_pTag = tag;
}

STDMETHODIMP_(void*) CApiSession::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CApiSession::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CApiSession::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    return S_OK;
}

STDMETHODIMP CApiSession::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return S_OK;
}

STDMETHODIMP CApiSession::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return S_OK;
}

//IEventCallback
STDMETHODIMP CApiSession::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    HRESULT hr;
    if(ET_Stream_Read == type)
    {
        IStream* pStream;
        JCHK(NULL != (pStream = (IStream*)source),E_INVALIDARG);
        if(0 < param1)
        {
            string json;
            json.resize(param1,0);
            JIF(pStream->Read((void*)json.data(),param1));

            Parser parser;

            Dynamic::Var param = parser.parse(json);
            Object::Ptr objParam = param.extract<Object::Ptr>();
            JIF(Convert(m_spProfile,objParam));
        }
        type = ET_Api_Push;
        hr = m_ep->Notify(type,0,(IFilter*)this,(void*)m_method.c_str());
    }
    else if(ET_Stream_Write == type)
    {
        hr = E_INVALIDARG;
    }
    return S_OK <= hr ? E_EOF : E_FAIL;
}

HRESULT CApiSession::SetType(FilterType type)
{
    JCHK(FT_Source == type || FT_Render == type,E_INVALIDARG);
    JCHK(FT_None == m_type,E_FAIL);

    HRESULT hr = S_OK;
    if(type == m_type)
        return hr;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);

    if(FT_Source == type)
    {
        JCHK(m_pinOut.Create(CLSID_COutputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinOut->SetMediaType(spMT));
    }
    else if(FT_Render == type)
    {
        JCHK(m_pinIn.Create(CLSID_CInputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinIn->SetMediaType(spMT));
    }

    m_type = type;
    return hr;
}

//HRESULT CApiSession::CreateBySession(uint32_t type,CMediaService* pService,IFilter* pFilter)
//{
//    JCHK(ET_Session_Push == type || ET_Session_Pull == type,E_INVALIDARG);
//    JCHK(NULL != pService,E_INVALIDARG);
//    JCHK(NULL != pFilter,E_INVALIDARG);
//
//    HRESULT hr = S_FALSE;
//    string name = pFilter->GetName();
//    size_t pos = name.find(API_PATH_NAME);
//    if(string::npos != pos)
//    {
//        uint32_t mode;
//        if(ET_Session_Push == type)
//            mode = FT_Render;
//        else
//            mode = FT_Source;
//
//        dom_ptr<ILoad> spLoad;
//        JCHK(NULL != (spLoad.p = (ILoad*)Create(IID(ILoad),NULL,false,pService)),E_FAIL);
//
//        JIF(spLoad->Load(name.c_str(),mode));
//
//        dom_ptr<IFilter> spFilter;
//        JCHK(spLoad.Query(&spFilter),E_FAIL);
//
//        dom_ptr<IOutputPin> spPinOut;
//        dom_ptr<IInputPin> spPinIn;
//
//        if(FT_Source == mode)
//        {
//            JCHK(spPinOut = spFilter->GetOutputPin(0),E_FAIL);
//            JCHK(spPinIn = pFilter->GetInputPin(0),E_FAIL);
//        }
//        else
//        {
//            JCHK(spPinOut = pFilter->GetOutputPin(0),E_FAIL);
//            JCHK(spPinIn = spFilter->GetInputPin(0),E_FAIL);
//        }
//        JIF(spPinOut->Connect(spPinIn));
//
//        IFilter* pUper;
//        IFilter* pDown;
//        JCHK(pUper = spPinOut->GetFilter(),E_FAIL);
//        JCHK(pDown = spPinIn->GetFilter(),E_FAIL);
//
//        JIF(pUper->Notify(IFilter::S_Pause));
//        JIF(pDown->Notify(IFilter::S_Pause));
//        JIF(pDown->Notify(IFilter::S_Play));
//        JIF(pUper->Notify(IFilter::S_Play));
//
//        JIF(pService->Append(spFilter));
//    }
//    return hr;
//}

HRESULT CApiSession::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL != format && 0 == strcmp(format,"api"))
        return 1;
    return E_INVALIDARG;
}
