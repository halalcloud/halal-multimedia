#include "BufSession.h"
#include <Locker.cpp>

CBufSession::CBufSession()
:m_callback(NULL)
,m_ansy(false)
,m_bound(false)
,m_stp_receive(0)
,m_pos_recevie(0)
,m_stp_deliver(0)
,m_pos_deliver(0)
{
    //ctor
}

bool CBufSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_callback = static_cast<IEpollCallback*>(pOuter),false);
    return true;
}

bool CBufSession::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        for(BufferIt it = m_bufs_receive.begin() ; it != m_bufs_receive.end() ; ++it)
            (*it)->Release();
        m_bufs_receive.clear();
        for(BufferIt it = m_bufs_deliver.begin() ; it != m_bufs_deliver.end() ; ++it)
            (*it)->Release();
        m_bufs_deliver.clear();
        m_spStream = NULL;
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CBufSession)
DOM_QUERY_IMPLEMENT(IBufSession)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CBufSession::Open(IStreamServer* pServer)
{
    HRESULT hr;
    JCHK(NULL != pServer,E_INVALIDARG);
    dom_ptr<IStream> spStream;
    JIF(pServer->Accept(&spStream,dynamic_cast<IEpollCallback*>(this)));
    m_spStream = spStream;
    return hr;
}

STDMETHODIMP CBufSession::Open(url* pUrl,int mode)
{
    HRESULT hr;
    JCHK(NULL != pUrl,E_INVALIDARG);
    dom_ptr<IStream> spStream;
    JCHK(spStream.Create(CLSID_CNetworkStream,dynamic_cast<IEpollCallback*>(this)),E_FAIL);
    JIF(spStream->Open(pUrl,mode));
    m_spStream = spStream;
    return hr;
}

STDMETHODIMP CBufSession::Receive(IBuffer** ppBuf)
{
    JCHK(NULL != ppBuf,E_INVALIDARG);
    JCHK(m_spStream != NULL,E_FAIL);

    CLocker locker(m_locker_read);
    JCHK(m_spStream->IsOpen(),E_FAIL);

    HRESULT hr = S_OK;
    BufferIt it = m_bufs_receive.begin();
    if(it != m_bufs_receive.end())
    {
        *ppBuf = *it;
        m_bufs_receive.erase(it);
    }
    else
    {
        if(NULL != m_buf_receive)
            return E_AGAIN;
        JCHK(m_buf_receive.Create(CLSID_CBuffer),E_FAIL);
        JIF(Read(m_buf_receive));
        JIF(m_buf_receive.CopyTo(ppBuf));
        m_buf_receive.p = NULL;
    }
    return hr;
}

STDMETHODIMP CBufSession::Deliver(IBuffer* pBuf)
{
    JCHK(NULL != pBuf,E_INVALIDARG);
    JCHK(m_spStream != NULL,E_FAIL);

    CLocker locker(m_locker_write);
    JCHK(m_spStream->IsOpen(),E_FAIL);

    HRESULT hr = S_OK;
    if(true == m_bufs_deliver.empty())
    {
        hr = Write(pBuf);
        if(E_AGAIN != hr)
        {
            if(S_OK < hr)
                hr = S_OK;
            return hr;
        }
    }
    m_bufs_deliver.push_back(pBuf);
    pBuf->Release();
    return m_bufs_deliver.size();
}

STDMETHODIMP_(bool) CBufSession::IsOpen()
{
    return m_spStream == NULL ? false : m_spStream->IsOpen();
}

STDMETHODIMP CBufSession::OnEvent(uint32_t id,void* pParam)
{
    HRESULT hr = S_OK;
    if(IStream::read == id)
    {
        if(m_buf_receive == NULL)
        {
            JCHK(m_buf_receive.Create(CLSID_CBuffer),E_FAIL);
        }
        JIF(Read(m_buf_receive));
        m_bufs_receive.push_back(m_buf_receive.p);
        m_buf_receive.p = NULL;
        if(NULL != m_callback)
            hr = m_callback->OnEvent(IBufSession::receive);
    }
    else if(IStream::write == id)
    {
        BufferIt it = m_bufs_deliver.begin();
        if(it != m_bufs_deliver.end())
        {
            IBuffer* pBuf = *it;
            JIF(Write(pBuf));
            m_bufs_deliver.erase(it);
            pBuf->Release();
        }
        if(true == m_bufs_deliver.empty())
        {
            return E_AGAIN;
        }
    }
    else
    {
        if(NULL != m_callback)
            hr = m_callback->OnEvent(id,pParam);
    }
    return hr;
}

STDMETHODIMP_(IEpoll*) CBufSession::GetEpoll()
{
    return NULL != m_callback ? m_callback->GetEpoll() : NULL;
}

HRESULT CBufSession::Read(uint8_t* pBuf,uint32_t len,uint32_t& pos)
{
    HRESULT hr;
    JCHK(m_spStream != NULL,E_FAIL);
    do
    {
        JIF(m_spStream->Read(pBuf + pos,len - pos));
        pos += hr;
    }while(pos < len);
    return hr;
}

HRESULT CBufSession::Write(uint8_t* pBuf,uint32_t len,uint32_t& pos)
{
    HRESULT hr;
    JCHK(m_spStream != NULL,E_FAIL);
    do
    {
        JIF(m_spStream->Write(pBuf + pos,len - pos));
        pos += hr;
    }while(pos < len);
    return hr;
}

HRESULT CBufSession::Read(IBuffer* pBuf)
{
    HRESULT hr = S_OK;
    switch(m_stp_receive)
    {
        case 0:
        {
            JIF(Read((uint8_t*)&pBuf->size,sizeof(pBuf->size),m_pos_recevie));
            m_pos_recevie = 0;
            ++m_stp_receive;
            JIF(pBuf->Alloc(pBuf->size));
        }
        case 1:
        {
            JIF(Read(pBuf->data,pBuf->size,m_pos_recevie));
            m_pos_recevie = 0;
            --m_stp_receive;
        }
    }
    return hr;
}

HRESULT CBufSession::Write(IBuffer* pBuf)
{
    HRESULT hr = S_OK;
    switch(m_stp_deliver)
    {
        case 0:
        {
            JIF(Write((uint8_t*)&pBuf->size,sizeof(pBuf->size),m_pos_deliver));
            m_pos_deliver = 0;
            ++m_stp_deliver;
        }
        case 1:
        {
            JIF(Write(pBuf->data,pBuf->size,m_pos_deliver));
            m_pos_deliver = 0;
            --m_stp_deliver;
        }
    }
    return hr;
}
