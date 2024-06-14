#include "file.h"
#include "epoll.h"


CFileStream::CFileStream()
:m_callback(NULL)
,m_tag(NULL)
{
}

bool CFileStream::FinalConstruct(Interface* pOuter,void* pParam)
{
    return true;
}

bool CFileStream::FinalDestructor(bool finally)
{
    Close();
    return true;
}

DOM_QUERY_IMPLEMENT_BEG(CFileStream)
DOM_QUERY_IMPLEMENT(IStream)
DOM_QUERY_IMPLEMENT(IEventCallbackSet)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(uint32_t) CFileStream::GetFlag()
{
    return m_stream.m_spEpoll == NULL ? SEEK_FLAG : ANSY_FLAG | SEEK_FLAG;
}

STDMETHODIMP CFileStream::Open(url* pUrl,int mode)
{
    JCHK(NULL != pUrl,E_INVALIDARG);
    JCHK(NULL != pUrl->path,E_INVALIDARG);

    Close();

    if(m_stream.m_spEpoll != NULL)
    {
        JCHK(NULL != m_callback,E_FAIL);
        mode |= O_NONBLOCK;
    }
    JCHK4(INVALID_FD != (m_stream.m_fd = open(pUrl->path,mode,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)),E_FAIL,
        "open file:%s flag:%d fail,error id:%d message:%s",pUrl->path,mode,errno,strerror(errno));
    m_stream.m_name = pUrl->path;
    return m_stream.Open(dynamic_cast<IEventCallback*>(this));
}

STDMETHODIMP_(void) CFileStream::Close()
{
    m_stream.Close();
}

STDMETHODIMP CFileStream::Read(void* pBuf,int32_t szBuf)
{
    return m_stream.Read((uint8_t*)pBuf,szBuf);
}

STDMETHODIMP CFileStream::Write(void* pBuf,int32_t szBuf)
{
    return m_stream.Write((uint8_t*)pBuf,szBuf);
}

STDMETHODIMP_(int64_t) CFileStream::Seek(int64_t position,seek_type type)
{
    return m_stream.Seek(position,type);
}

STDMETHODIMP_(int64_t) CFileStream::GetLength()
{
    int64_t pos = Seek(0,IStream::current);
    int64_t len = Seek(0,IStream::end);
    Seek(pos,IStream::begin);
    return len;
}

STDMETHODIMP CFileStream::SetLength(int64_t len)
{
    return E_FAIL;
}

STDMETHODIMP_(bool) CFileStream::IsOpen()
{
    return INVALID_FD != m_stream.m_fd;
}

STDMETHODIMP_(void) CFileStream::SetCallback(IEventCallback* pCallback,void* pTag)
{
    m_callback = pCallback;
    m_tag = pTag;
}

STDMETHODIMP CFileStream::OnEvent(uint32_t id,void* pParam,void* pTag)
{
    JCHK(NULL != m_callback,E_FAIL);

    HRESULT hr = S_OK;

    if(0 != (EPOLLERR&id))
    {
        hr = m_callback->OnEvent(et_error,NULL,m_tag);
    }
    else if(0 != (EPOLLIN&id))
    {
        hr = m_callback->OnEvent(IStream::read,NULL,m_tag);
    }
    else if(EPOLLOUT == id)
    {
        hr = m_callback->OnEvent(IStream::write,NULL,m_tag);
    }
    return hr;
}
