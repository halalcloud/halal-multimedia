#include "OutputPin.h"
#include <stdio.h>
COutputPin::COutputPin()
:m_pFilter(NULL)
,m_index(PIN_INVALID_INDEX)
,m_pTag(NULL)
,m_pos(MEDIA_FRAME_NONE_TIMESTAMP)
,m_is_new_segment(false)
#ifdef PIN_OUTPUT
,m_dump(NULL)
#endif
{
    m_it = m_connections.end();
}

bool COutputPin::FinalConstruct(Interface* pOuter,void* pParam)
{
    if(NULL != pOuter)
        m_pFilter = static_cast<IFilter*>(pOuter);
    else
        m_pFilter = NULL;
    size_t* pIndex;
    if(NULL != (pIndex = (size_t*)pParam))
        m_index = *pIndex;
    return m_allocate.Create(CLSID_CMediaFrameAllocate);
}

bool COutputPin::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Disconnect(NULL);
    #ifdef PIN_OUTPUT
        if(NULL != m_dump)
        {
            fclose(m_dump);
            m_dump = NULL;
        }
    #endif
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(COutputPin)
DOM_QUERY_IMPLEMENT(IOutputPin)
DOM_QUERY_IMPLEMENT_END

//IPin
STDMETHODIMP_(IFilter*) COutputPin::GetFilter()
{
    return m_pFilter;
}

STDMETHODIMP COutputPin::GetMediaType(IMediaType* pMT)
{
    JCHK(NULL != pMT,E_INVALIDARG);
    return m_spMT == NULL ? m_pFilter->OnGetMediaType(this,pMT) : pMT->CopyFrom(m_spMT);
}

STDMETHODIMP_(IMediaType*) COutputPin::GetMediaType()
{
    return m_spMT;
}

STDMETHODIMP COutputPin::SetIndex(uint32_t index)
{
    m_index = index;
    return S_OK;
}

STDMETHODIMP_(uint32_t) COutputPin::GetIndex()
{
    return m_index;
}

STDMETHODIMP COutputPin::Write(IMediaFrame* pFrame)
{
#ifdef PIN_OUTPUT
    if(NULL != m_dump)
    {
        size_t szBuf;
        void* pBuf = pFrame->GetBuf(&szBuf);
        if(NULL != pBuf && 0 < szBuf)
        {
            fwrite(pBuf,1,szBuf,m_dump);
        }
    }
#endif
    if(NULL != pFrame)
    {
        if(true == m_is_new_segment)
        {
            pFrame->info.flag |= MEDIA_FRAME_FLAG_NEWSEGMENT;
            m_is_new_segment = false;
        }
        else
            pFrame->info.flag &= ~MEDIA_FRAME_FLAG_NEWSEGMENT;

        if(MEDIA_FRAME_NONE_TIMESTAMP != m_pos && pFrame->info.dts < m_pos)
        {
            LOG(0,"output pin frame dts:%dms exception,previous:%dms",int(pFrame->info.dts/10000),int(m_pos/10000));
            pFrame->info.dts = m_pos + pFrame->info.duration;
        }
        m_pos = pFrame->info.dts;
    }

    HRESULT hr = S_OK;
    hr = m_pFilter->OnNotify(IFilterEvent::Process,hr,NULL,this,pFrame);
    if(S_OK != hr)
        return hr;

    hr = S_STREAM_EOF;
    for(m_it = m_connections.begin() ; m_it !=  m_connections.end();)
    {
        ConnectionIt it = m_it;
        HRESULT result = (*it)->Write(pFrame);
        if(S_OK > result)
        {
            hr = result;
            break;
        }
        else if(S_STREAM_EOF != result)
        {
            if(S_OK != hr)
                hr = result;
        }
        if(it == m_it)
            ++m_it;
    }
    return hr;
}

STDMETHODIMP COutputPin::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) COutputPin::GetTag()
{
    return m_pTag;
}
//IOutputPin
STDMETHODIMP COutputPin::Connect(IInputPin* pPin,IMediaType* pMT)
{
    HRESULT hr;
    JCHK(NULL != pPin,E_INVALIDARG);
    if(m_spMT == NULL)
    {
        dom_ptr<IMediaType> spMT;
        if(NULL == pMT)
        {
            JIF(spMT.Create(CLSID_CMediaType));
            JIF(m_pFilter->OnGetMediaType(this,spMT));
            pMT = spMT.p;
        }
        JIF(m_pFilter->OnSetMediaType(this,pMT));
        JIF(pPin->Connect(this,pMT));
        m_spMT = pMT;
    }
    else
    {
        if(pMT != NULL)
        {
            JIF(m_spMT->Compare(pMT));
            if(S_OK != hr)
                return E_INVALIDARG;
        }
        JIF(pPin->Connect(this,m_spMT));
    }
    return hr;
}

STDMETHODIMP COutputPin::Disconnect(IInputPin* pPin)
{
    if(NULL != pPin)
    {
        pPin->Disconnect();
    }
    else
    {
        ConnectionIt it;
        while(m_connections.end() != (it = m_connections.begin()))
        {
            it->p->Disconnect();
        }
    }
    if(m_connections.empty())
    {
        if(NULL != m_pFilter)
            m_pFilter->OnSetMediaType(this,NULL);
#ifdef PIN_OUTPUT
        if(NULL != m_dump)
        {
            fclose(m_dump);
            m_dump = NULL;
        }
#endif
    }
    return S_OK;
}

STDMETHODIMP_(IInputPin*) COutputPin::GetConnection()
{
    return true == m_connections.empty() ? NULL : m_connections.front().p;
}

STDMETHODIMP_(void) COutputPin::NewSegment()
{
    m_is_new_segment = true;
}

STDMETHODIMP COutputPin::AllocFrame(IMediaFrame** ppFrame)
{
    return m_allocate->Alloc(ppFrame);
}

ConnectionIt COutputPin::Connect(IInputPin* pPin)
{
    return m_connections.insert(m_connections.end(),pPin);
}

void COutputPin::Disconnect(ConnectionIt& it)
{
    ConnectionIt itTemp = m_connections.erase(it);
    if(it == m_it)
        m_it = itTemp;
    it = m_connections.end();
}

IInputPin* COutputPin::Next(ConnectionIt it)
{
    return ++it == m_connections.end() ? NULL : it->p;
}
