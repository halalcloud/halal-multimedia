#include "MediaFrame.h"
#include <malloc.h>

CMediaFrame::CMediaFrame()
:m_pAllocate(NULL)
,m_pBuf(NULL)
,m_szBuf(0)
,m_szUse(0)
{
    memset(&info,0,sizeof(info));
}

bool CMediaFrame::FinalConstruct(Interface* pOuter,void* pParam)
{
    m_pAllocate = static_cast<CMediaFrameAllocate*>(pOuter);
    if(NULL != m_pAllocate)
        m_pAllocate->Init(this);
    return true;
}

bool CMediaFrame::FinalDestructor(bool finally)
{
    if(false == finally)
    {
        if(NULL != m_pAllocate)
            finally = m_pAllocate->Free(this);
    }
    if(true == finally)
        Clear();
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CMediaFrame)
DOM_QUERY_IMPLEMENT(IMediaFrame)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CMediaFrame::Alloc(uint32_t size)
{
    JCHK(0 < size,E_INVALIDARG);
    if(m_szBuf != size)
    {
        JCHK(m_pBuf = (uint8_t*)realloc(m_pBuf,size),E_OUTOFMEMORY);
        m_szBuf = size;
    }
    m_szUse = 0;
    return S_OK;
}

STDMETHODIMP_(uint8_t*) CMediaFrame::GetBuf(uint32_t* pSize)
{
    if(NULL != pSize)
        *pSize = m_szUse;
    return m_pBuf;
}

STDMETHODIMP CMediaFrame::SetBuf(uint8_t* pBuf,uint32_t lenBuf)
{
    JCHK(0 < lenBuf,E_INVALIDARG);
    if(0 < m_szBuf)
    {
        JCHK(lenBuf <= m_szBuf,E_INVALIDARG);
        if(NULL != pBuf && pBuf != m_pBuf)
            memcpy(m_pBuf,pBuf,lenBuf);
    }
    else
    {
        JCHK(NULL != pBuf,E_INVALIDARG);
        m_pBuf = pBuf;
    }
    m_szUse = lenBuf;
    return S_OK;
}

STDMETHODIMP CMediaFrame::CopyTo(IMediaFrame* pFrame,uint32_t flag)
{
    return Copy(pFrame,this,flag);
}

STDMETHODIMP CMediaFrame::CopyFrom(IMediaFrame* pFrame,uint32_t flag)
{
    return Copy(this,pFrame,flag);
}

STDMETHODIMP_(void) CMediaFrame::Clear()
{
    if(0 < m_szBuf)
    {
        if(NULL != m_pBuf)
            free(m_pBuf);
        m_szBuf = 0;
    }
    m_pBuf = NULL;
    m_szUse = 0;
}

IMediaFrame* CMediaFrame::Uses()
{
    if(NULL != m_pAllocate)
    {
        m_pAllocate->Uses(this);
        AddRef();
        return dynamic_cast<IMediaFrame*>(this);
    }
    else
        return NULL;
}

HRESULT CMediaFrame::Copy(IMediaFrame* pDest,IMediaFrame* pSour,uint32_t flag)
{
    HRESULT hr = S_OK;
    uint32_t szBuf;
    JCHK(NULL != pDest,E_INVALIDARG);
    JCHK(NULL != pSour,E_INVALIDARG);
    if(0 != (flag & MEDIAFRAME_COPY_INFO))
    {
        pDest->info = pSour->info;
    }
    if(0 != (flag & MEDIAFRAME_COPY_DATA))
    {
        uint8_t* pBuf = pSour->GetBuf(&szBuf);
        if(NULL == pBuf || 0 == szBuf)
            pDest->Clear();
        else
        {
            JIF(pDest->Alloc(szBuf));
            JIF(pDest->SetBuf(pBuf,szBuf));
        }
    }
    return hr;
}

CMediaFrameAllocate::CMediaFrameAllocate()
{
}

bool CMediaFrameAllocate::FinalConstruct(Interface* pOuter,void* pParam)
{
    return true;
}

bool CMediaFrameAllocate::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear(true);
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CMediaFrameAllocate)
DOM_QUERY_IMPLEMENT(IMediaFrameAllocate)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CMediaFrameAllocate::Alloc(IMediaFrame** ppFrame)
{
	JCHK(NULL != ppFrame,E_INVALIDARG);
    FrameIt it = m_free.begin();
	if(it == m_free.end())
	{
        IMediaFrame* pFrame;
        JCHK(pFrame = static_cast<IMediaFrame*>(CMediaFrame::Create(IID(IMediaFrame),this)),E_FAIL);
        pFrame->AddRef();
        *ppFrame = pFrame;
	}
	else
	{
        CMediaFrame* pFrame;
        JCHK(pFrame = *it,E_FAIL);
        JCHK(*ppFrame = pFrame->Uses(),E_FAIL);
	}
	return S_OK;
}

STDMETHODIMP_(void) CMediaFrameAllocate::Clear()
{
    Clear(false);
}

void CMediaFrameAllocate::Init(CMediaFrame* pFrame)
{
    pFrame->m_it = m_uses.insert(m_uses.end(),pFrame);
}

void CMediaFrameAllocate::Uses(CMediaFrame* pFrame)
{
    m_free.erase(pFrame->m_it);
    pFrame->m_it = m_uses.insert(m_uses.end(),pFrame);
}

bool CMediaFrameAllocate::Free(CMediaFrame* pFrame)
{
    m_uses.erase(pFrame->m_it);
    if(true == m_free.empty())
    {
        pFrame->m_it = m_free.insert(m_free.end(),pFrame);
        return false;
    }
    else
        return true;
}

void CMediaFrameAllocate::Clear(bool isRelease)
{
	CMediaFrame* pFrame;
	FrameIt it = m_free.begin();
	while(it != m_free.end())
	{
		if(NULL != (pFrame = *it))
            pFrame->Release();
		it = m_free.erase(it);
	}
	if(true == isRelease)
	{
        it = m_uses.begin();
        while(it != m_uses.end())
        {
            if(NULL != (pFrame = *it))
                pFrame->Release();
            it = m_uses.erase(it);
        }
	}
}
