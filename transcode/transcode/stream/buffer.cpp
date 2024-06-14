#include "buffer.h"

CBuffer::CBuffer()
:m_data(NULL)
,m_size(0)
,m_pos_write(0)
,m_pos_read(0)
{
    //ctor
    IBuffer::data = NULL;
    IBuffer::size = 0;
}

bool CBuffer::FinalConstruct(Interface* pOuter,void* pParam)
{
    return true;
}

bool CBuffer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CBuffer)
DOM_QUERY_IMPLEMENT(IBuffer)
DOM_QUERY_IMPLEMENT(IStream)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CBuffer::Alloc(uint32_t size)
{
    JCHK(0 < size,E_INVALIDARG);
    if(size != m_size)
    {
        JCHK(m_data = (uint8_t*)realloc(m_data,size),E_OUTOFMEMORY);
        m_size = size;
    }
    IBuffer::data = m_data;
    IBuffer::size = m_size;
    m_pos_read = 0;
    m_pos_write = 0;
    return S_OK;
}

STDMETHODIMP_(uint32_t) CBuffer::GetSize()
{
    return m_size;
}

STDMETHODIMP_(void) CBuffer::Clear()
{
    if(NULL != m_data)
    {
        free(m_data);
        m_data = NULL;
    }
    m_size = 0;
    IBuffer::data = NULL;
    IBuffer::size = 0;
    m_pos_read = 0;
    m_pos_write = 0;
}

STDMETHODIMP CBuffer::CopyFrom(IBuffer* pBuf)
{
    return Copy(this,pBuf);
}

STDMETHODIMP CBuffer::CopyTo(IBuffer* pBuf)
{
    return Copy(pBuf,this);
}

STDMETHODIMP_(uint32_t) CBuffer::GetFlag()
{
    return 0;
}

STDMETHODIMP CBuffer::Open(url* pUrl,int mode)
{
    IBuffer::data = NULL;
    IBuffer::size = 0;
    m_pos_read = 0;
    m_pos_write = 0;
    return S_OK;
}

STDMETHODIMP_(void) CBuffer::Close()
{
    IBuffer::data = NULL;
    IBuffer::size = 0;
    m_pos_read = 0;
    m_pos_write = 0;
}

STDMETHODIMP CBuffer::Read(void* pBuf,int32_t szBuf)
{
    JCHK(m_pos_read + szBuf <= m_size,E_INVALIDARG);
    memcpy(pBuf,m_data+m_pos_read,szBuf);
    m_pos_read += szBuf;
    IBuffer::size = m_pos_read;
    return szBuf;
}

STDMETHODIMP CBuffer::Write(void* pBuf,int32_t szBuf)
{
    if(NULL != IBuffer::data)
    {
        JCHK(m_pos_write + szBuf <= m_size,E_INVALIDARG);
        memcpy(m_data + m_pos_write,pBuf,szBuf);
        m_pos_write += szBuf;
        IBuffer::size = m_pos_write;
    }
    else
        IBuffer::size += szBuf;
    return szBuf;
}

STDMETHODIMP_(int64_t) CBuffer::Seek(int64_t position,seek_type type)
{
    return E_FAIL;
}

STDMETHODIMP_(int64_t) CBuffer::GetLength()
{
    return m_size;
}

STDMETHODIMP CBuffer::SetLength(int64_t len)
{
    return Alloc((uint32_t)len);
}

STDMETHODIMP_(bool) CBuffer::IsOpen()
{
    return NULL != m_data;
}

HRESULT CBuffer::Copy(IBuffer* pDest,IBuffer* pSour)
{
    HRESULT hr;
    JCHK(NULL != pDest,E_INVALIDARG);
    JCHK(NULL != pSour,E_INVALIDARG);
    if(pDest == pSour)
        return S_OK;
    JIF(pDest->Alloc(pSour->GetSize()));
    memcpy(pDest->data,pSour->data,pSour->size);
    pDest->size = pSour->size;
    return hr;
}

