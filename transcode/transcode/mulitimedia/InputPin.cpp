#include "InputPin.h"

CInputPin::CInputPin()
:m_pFilter(NULL)
,m_index(PIN_INVALID_INDEX)
,m_pTag(NULL)
,m_pPin(NULL)
{
    //ctor
}

bool CInputPin::FinalConstruct(Interface* pOuter,void* pParam)
{
    if(NULL != pOuter)
        m_pFilter = static_cast<IFilter*>(pOuter);
    else
        m_pFilter = NULL;

    size_t* pIndex;
    if(NULL != (pIndex = (size_t*)pParam))
        m_index = *pIndex;
    return true;
}

bool CInputPin::FinalDestructor(bool finally)
{
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CInputPin)
DOM_QUERY_IMPLEMENT(IInputPin)
DOM_QUERY_IMPLEMENT_END

//IPin
STDMETHODIMP_(IFilter*) CInputPin::GetFilter()
{
    return m_pFilter;
}

STDMETHODIMP CInputPin::GetMediaType(IMediaType* pMT)
{
    JCHK(NULL != pMT,E_INVALIDARG);
    return m_spMT == NULL ? m_pFilter->OnGetMediaType(this,pMT) : pMT->CopyFrom(m_spMT);
}

STDMETHODIMP_(IMediaType*) CInputPin::GetMediaType()
{
    return m_spMT;
}

STDMETHODIMP CInputPin::SetIndex(uint32_t index)
{
    m_index = index;
    return S_OK;
}

STDMETHODIMP_(uint32_t) CInputPin::GetIndex()
{
    return m_index;
}

STDMETHODIMP CInputPin::Write(IMediaFrame* pFrame)
{
    HRESULT hr = S_OK;
    hr = m_pFilter->OnNotify(IFilterEvent::Process,hr,this,NULL,pFrame);
    return S_OK == hr ? m_pFilter->OnWriteFrame(this,pFrame) : hr;
}

STDMETHODIMP CInputPin::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CInputPin::GetTag()
{
    return m_pTag;
}


//IInputPin
STDMETHODIMP CInputPin::Connect(IOutputPin* pPin,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != pMT,E_INVALIDARG);
    JCHK(NULL == m_pPin,E_FAIL);
    JCHK(m_spMT == NULL,E_FAIL);

    HRESULT hr;
    JIF(m_pFilter->OnSetMediaType(this,pMT));
    m_spMT = pMT;
    m_pPin = static_cast<COutputPin*>(pPin);
    m_it = m_pPin->Connect(this);
    return hr;
}

STDMETHODIMP_(void) CInputPin::Disconnect()
{
    if(NULL == m_pPin)
        return;

    if(NULL != m_pFilter)
    {
        m_pFilter->OnSetMediaType(this,NULL);
    }
    COutputPin* pPin = m_pPin;
    m_spMT = NULL;
    m_pPin = NULL;
    pPin->Disconnect(m_it);
}

STDMETHODIMP_(IOutputPin*) CInputPin::GetConnection()
{
    return m_pPin;
}

STDMETHODIMP_(IInputPin*) CInputPin::Next()
{
    return NULL == m_pPin ? NULL : m_pPin->Next(m_it);
}

