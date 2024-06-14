#include "HttpStream.h"

CHttpStream::CHttpStream()
:m_callback(NULL)
,m_tag(NULL)
{
    //ctor
}

bool CHttpStream::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_tcp.Create(CLSID_CNetworkStream,NULL,pParam),false);
    dom_ptr<IEventCallbackSet> spCallbackSet;
    JCHK(m_tcp.Query(&spCallbackSet),false);
    spCallbackSet->SetCallback(dynamic_cast<IEventCallback*>(this));
    return true;
}

void CHttpStream::FinalDestructor(bool finally)
{
}

DOM_QUERY_IMPLEMENT_BEG(CHttpStream)
DOM_QUERY_IMPLEMENT(IStream)
DOM_QUERY_IMPLEMENT(IEventCallbackSet)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(uint32_t) CHttpStream::GetFlag()
{
    return m_tcp->GetFlag() | SEEK_FLAG;
}

STDMETHODIMP CHttpStream::Open(url* pUrl,int mode)
{
    return m_tcp->Open(pUrl,mode);
}

STDMETHODIMP_(void) CHttpStream::Close()
{
    m_tcp->Close();
}

STDMETHODIMP CHttpStream::Read(void* pBuf,int32_t szBuf)
{
    return S_OK;
}

STDMETHODIMP CHttpStream::Write(void* pBuf,int32_t szBuf)
{
    return S_OK;
}

STDMETHODIMP_(int64_t) CHttpStream::Seek(int64_t position,seek_type type)
{
    return S_OK;
}

STDMETHODIMP_(int64_t) CHttpStream::GetLength()
{
    return 0;
}

STDMETHODIMP CHttpStream::SetLength(int64_t len)
{
    return 0;
}

STDMETHODIMP_(bool) CHttpStream::IsOpen()
{
    return false;
}

STDMETHODIMP_(void) CHttpStream::SetCallback(IEventCallback* pCallback,void* pTag)
{
    m_callback = pCallback;
    m_tag = pTag;
}

STDMETHODIMP CHttpStream::OnEvent(uint32_t id,void* pParam,void* pTag)
{
    return S_OK;
}
