#include "FileSession.h"
#include <Url.cpp>
const int DEFAULT_BUFFER_SIZE = 1024;
CFileSession::CFileSession()
:m_type(FT_None)
,m_flag(0)
,m_status(S_Stop)
,m_pTag(NULL)
,m_len(0)
,m_lenBuf(DEFAULT_BUFFER_SIZE)
{
}

bool CFileSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    JCHK(m_spStream.Create(CLSID_CFileStream,(ICallback*)this),false);
    return true;
}

bool CFileSession::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Notify(S_Stop);
        LOG(0,"%s[%p] name:%s release",Class().name,(IFilter*)this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFileSession)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CFileSession::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    HRESULT hr;
    if(NULL != pUrl)
        m_name = pUrl;
    JIF(SetType((FilterType)mode));
    if(FT_Source == (FilterType)mode)
    {
        JIF(m_ep->Notify(ET_Filter_Build,0,(IFilter*)this));
    }
    return hr;
}

STDMETHODIMP_(FilterType) CFileSession::GetType()
{
    return m_type;
}

STDMETHODIMP CFileSession::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFileSession::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFileSession::GetFlag()
{
    return m_spStream == NULL ? m_flag : m_flag | m_spStream ->GetFlag();
}

STDMETHODIMP_(uint32_t) CFileSession::GetInputPinCount()
{
    return m_pinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CFileSession::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn.p : NULL;
}

STDMETHODIMP_(uint32_t) CFileSession::GetOutputPinCount()
{
    return m_pinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CFileSession::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut.p : NULL;
}

STDMETHODIMP_(IInputPin*) CFileSession::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFileSession::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFileSession::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(S_Stop == m_status)
            {
                JIF(m_spStream->Open(m_name.c_str(),(uint32_t)m_type));
                if(m_pinOut != NULL)
                {
                    IMediaType* pMT;
                    JCHK(pMT = m_pinOut->GetMediaType(),E_FAIL);
                    m_len = m_spStream->GetLength();
                    JIF(pMT->SetStreamInfo(NULL,&m_len));
                }
            }
            else if(S_Stop == cmd)
            {
                m_spStream->Close();
            }
            else if(m_spStream->IsOpen())
            {
//                if(S_Play == cmd)
//                {
//                    JIF(m_spStream->SetEventEnable(true));
//                }
//                else
//                {
//                    JIF(m_spStream->SetEventEnable(false));
//                }
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
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CFileSession::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFileSession::SetTag(void* tag)
{
    m_pTag = tag;
}

STDMETHODIMP_(void*) CFileSession::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFileSession::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CFileSession::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    if(NULL != pMT)
        return MMT_DATA == pMT->GetMajor() ? S_OK : E_INVALIDARG;
    else
        return S_OK;
}

STDMETHODIMP CFileSession::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    if(NULL != pMT)
        return MMT_DATA == pMT->GetMajor() ? S_OK : E_INVALIDARG;
    else
        return S_OK;
}

STDMETHODIMP CFileSession::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    if(NULL != pFrame)
        return m_spStream->Write(pFrame,0,IStream::WRITE_FLAG_FRAME);
    else
        return E_EOF;
}

//IEventCallback
STDMETHODIMP CFileSession::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    HRESULT hr;
    if(ET_Stream_Read == type)
    {
        dom_ptr<IMediaFrame> spFrame;
        JIF(m_pinOut->AllocFrame(&spFrame));
        JIF(spFrame->SetBuf((uint32_t)0,m_lenBuf));
        IMediaFrame::buf* buf = (IMediaFrame::buf*)spFrame->GetBuf();
        JIF(m_spStream->Read(buf->data,buf->size));
        if((uint32_t)hr < buf->size)
            buf->size = (uint32_t)hr;
        spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
        if(m_spStream->GetStatus().read_total_size >= (uint64_t)m_len)
            spFrame->info.flag |= MEDIA_FRAME_FLAG_EOF;
        return m_pinOut->Write(spFrame);
    }
    else if(ET_Stream_Write == type)
    {
        if(FT_Render == m_type)
        {
            if(NULL == m_pinIn)
                return E_AGAIN;
            JIF(m_pinIn->Write(NULL));
            type = ET_Filter_Buffer;
            param2 = m_pinIn.p;
        }
        else if(FT_Source == m_type)
        {
            JIF(m_spStream->SetEventEnable(true));
            return E_AGAIN;
        }
    }
    else if(ET_Filter_Render == type)
    {
        JCHK(FT_Source == m_type,E_FAIL);
        if(0 == param1)
        {
            JIF(m_spStream->SetEventEnable(false));
        }
        return param1;
    }
    return m_ep->Notify(type,param1,param2,param3);
}

HRESULT CFileSession::SetType(FilterType type)
{
    JCHK(FT_Source == type || FT_Render == type,E_INVALIDARG);
    JCHK(FT_None == m_type,E_FAIL);

    HRESULT hr = S_OK;
    if(type == m_type)
        return hr;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
    JIF(spMT->SetMajor(MMT_DATA));
    size_t pos = m_name.find_last_of('.');
    if(pos != string::npos)
    {
        string ext = m_name.substr(pos+1);
        spMT->SetSub(ext.c_str());
    }
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

HRESULT CFileSession::Init()
{
    m_spProfile->Read("buffer_size",m_lenBuf);
    if(0 >= m_lenBuf)
        m_lenBuf = DEFAULT_BUFFER_SIZE;
    return S_OK;
}

HRESULT CFileSession::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if((NULL == protocol || 0 == strlen(protocol)) && (NULL == format || 0 != strcmp(format,"api")))
        return 1;
    return E_INVALIDARG;
}


