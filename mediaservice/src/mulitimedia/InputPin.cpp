#include "InputPin.h"

CInputPin::CInputPin()
:m_pFilter(NULL)
,m_index(PIN_INVALID_INDEX)
,m_id(PIN_INVALID_INDEX)
,m_pTag(NULL)
,m_pPin(NULL)
,m_cmd(Filter_CMD_None)
,m_is_end(false)
,m_len(0)
,m_key(0)
,m_flag(0)
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
    else
        m_index = 0;
    return true;
}

bool CInputPin::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        m_frames.clear();
        m_spMT = NULL;
    }
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

STDMETHODIMP_(IMediaType*) CInputPin::GetMediaType()
{
    return m_spMT;
}

STDMETHODIMP CInputPin::SetMediaType(IMediaType* pMT)
{
    if(NULL != pMT)
    {
        m_len = 0;

        dom_ptr<IProfile> spProfile;
        JCHK(spProfile.QueryFrom(pMT),E_INVALIDARG);

        IProfile::val* pVal = spProfile->Read("buffer");
        if(pVal != NULL)
        {
            if(true == STR_CMP(pVal->type,typeid(int).name()))
                m_len = *(int*)pVal->value;
            else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
                true == STR_CMP(pVal->type,typeid(char*).name()))
            {
                char* pBeg = (char*)pVal->value;
                char* pEnd = pBeg;
                int duration = strtol(pBeg,&pEnd,10);
                if(pEnd > pBeg && 0 <= duration)
                {
                    if(0 < duration)
                    {
                        if('s' == *pEnd)
                        {
                            m_len = duration * 10000000;
                        }
                        else if(0 == strcmp("ms",pEnd))
                        {
                            m_len = duration * 10000;
                        }
                        else
                        {
                            m_len = duration;
                        }
                    }
                }
            }
        }
    }
    m_spMT = pMT;
    return S_OK;
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

STDMETHODIMP_(void) CInputPin::SetID(uint32_t id)
{
    m_id = id;
}

STDMETHODIMP_(uint32_t) CInputPin::GetID()
{
    return m_id;
}

STDMETHODIMP CInputPin::Send(uint32_t cmd,bool down,bool first)
{
    JCHK(Filter_CMD_None != cmd,E_INVALIDARG);

    HRESULT hr = S_OK;
    if(true == down)
    {
        m_cmd = cmd;

        uint32_t count = m_pFilter->GetInputPinCount();
        for(uint32_t i=0 ; i<count ; ++i)
        {
            if(i != m_index)
            {
                IInputPin* pPin = m_pFilter->GetInputPin(i);
                if(NULL != pPin->GetConnection())
                {
                    if(cmd != pPin->Recv())
                        return S_OK;
                }
            }
        }

        for(uint32_t i=0 ; i<count ; ++i)
        {
            IInputPin* pPin = m_pFilter->GetInputPin(i);
            if(NULL != pPin->GetConnection())
            {
                pPin->Set(Filter_CMD_None);
            }
        }

        if(true == first)
        {
            JIF(m_pFilter->Notify(cmd));
            if(S_OK != hr)
                return hr;
        }
        bool is_ok = true;
        count = m_pFilter->GetOutputPinCount();
        for(uint32_t i=0 ; i<count ; ++i)
        {
            IOutputPin* pPin = m_pFilter->GetOutputPin(i);
            if(NULL != pPin->GetConnection())
            {
                JIF(pPin->Send(cmd,down,first));
                if(S_OK != hr)
                    is_ok = false;
            }
        }
        if(false == first)
        {
            if(true == is_ok)
            {
                JIF(m_pFilter->Notify(cmd));
            }
            else
                hr = S_FALSE;
        }
        else
            hr = true == is_ok ? S_OK : S_FALSE;
    }
    else
    {
        if(m_pPin != NULL)
        {
            if(false == first)
                m_cmd = cmd;

            JIF(m_pPin->Send(cmd,down,first));

            if(true == first)
                m_cmd = cmd;
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CInputPin::Recv()
{
    return m_cmd;
}

STDMETHODIMP_(void) CInputPin::Set(uint32_t cmd)
{
    m_cmd = cmd;
}

STDMETHODIMP CInputPin::Write(IMediaFrame* pFrame)
{
    if(NULL != pFrame)
    {
        if(IFilter::S_Play != m_pFilter->GetStatus())
            return S_FALSE;

        if(0 != m_flag)
        {
            if(0 != (m_flag & MEDIA_FRAME_FLAG_SYNCPOINT))
            {
                if(0 == (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
                    return S_OK;
            }
            if(pFrame->info.flag != m_flag)
                pFrame->info.flag |= m_flag;
            m_flag = 0;
        }
        m_is_end = 0 != (pFrame->info.flag&MEDIA_FRAME_FLAG_EOF);
    }
    else
    {
        if(IFilter::S_Play != m_pFilter->GetStatus())
            return E_AGAIN;
    }

    HRESULT hr = S_OK;
    if(0 != m_len)
    {
        if(NULL == m_pPin)
            return E_AGAIN;

        if(NULL != pFrame)
        {
            CLocker locker(m_locker);
            JIF(FrameBufferInput(m_frames,pFrame,m_len,m_key));
            //LOG(0,"stream:%s frame buffer size:%d",m_spMT->GetSubName(),m_frames.size());
        }

        while(true)
        {
            IInputPin* pPinResult = NULL;
            dom_ptr<IMediaFrame> spFrameResult;
            uint32_t count = m_pFilter->GetInputPinCount();
            for(uint32_t i=0 ; i<count ; ++i)
            {
                IInputPin* pPin = m_pFilter->GetInputPin(i);
                if(NULL != pPin->GetConnection())
                {
                    dom_ptr<IMediaFrame> spFrame;
                    hr = pPin->Pop(&spFrame);
                    if(0 < hr)
                    {
                        if(spFrameResult == NULL || spFrame->info.dts < spFrameResult->info.dts)
                        {
                            spFrameResult = spFrame;
                            pPinResult = pPin;
                        }
                    }
                    else if(0 == hr)
                        return hr;
                }
            }
            if(NULL != pPinResult)
            {
                hr = m_pFilter->OnWriteFrame(pPinResult,spFrameResult);
                if(S_OK > hr)
                {
                    if(E_AGAIN == hr && NULL != pFrame)
                        return S_OK;
                    if(E_EOF == hr && 0 != (spFrameResult->info.flag&MEDIA_FRAME_FLAG_EOF))
                        pPinResult->Pop();
                }
                else
                {
                    pPinResult->Pop();
                    if(0 == hr && NULL == pFrame)
                        return E_AGAIN;
                }
            }
            else
                hr = E_EOF;
        }
    }
    else
    {
        if(NULL != pFrame)
            hr = m_pFilter->OnWriteFrame(this,pFrame);
    }
    return hr;
}

STDMETHODIMP_(void) CInputPin::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CInputPin::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(void) CInputPin::SetObj(Interface* pObj)
{
    m_obj = pObj;
}

STDMETHODIMP_(Interface*) CInputPin::GetObj()
{
    return m_obj;
}

STDMETHODIMP CInputPin::SetFlag(uint8_t flag)
{
    JCHK(0 != flag,E_INVALIDARG);
    m_flag = flag;
    return S_OK;
}

//IInputPin
STDMETHODIMP CInputPin::Connect(IOutputPin* pPin,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != pMT,E_INVALIDARG);
    JCHK(NULL == m_pPin,E_FAIL);

    HRESULT hr;

    CLocker locker(m_locker);

    JIF(m_pFilter->OnConnect(this,pPin,pMT))
    m_pPin = static_cast<COutputPin*>(pPin);
    m_it = m_pPin->Connect(this);
    m_spMT = pMT;
    return hr;
}

STDMETHODIMP_(void) CInputPin::Disconnect()
{
    CLocker locker(m_locker);

    if(NULL == m_pPin)
        return;
    m_pFilter->OnConnect(this,m_pPin,NULL);
    COutputPin* pPin = m_pPin;
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

STDMETHODIMP CInputPin::Pop(IMediaFrame** ppFrame)
{
    CLocker locker(m_locker);
    FrameIt it = m_frames.begin();
    if(NULL != ppFrame)
    {
        if(it != m_frames.end())
        {
            (*it).CopyTo(ppFrame);
            return m_frames.size();
        }
        else
            return true == m_is_end ? E_EOF : 0;
    }
    else
    {
        if(m_frames.end() != it)
        {
            dom_ptr<IMediaFrame>& spFrame = *it;
            if(0 != (spFrame->info.flag&MEDIA_FRAME_FLAG_SYNCPOINT))
                --m_key;
            m_frames.erase(it);
        }
        return m_frames.size();
    }
}

STDMETHODIMP_(int64_t) CInputPin::GetBufLen()
{
    return m_len;
}

STDMETHODIMP_(bool) CInputPin::IsEnd()
{
    return m_is_end;
}
