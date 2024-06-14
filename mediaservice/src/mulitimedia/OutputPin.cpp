#include "OutputPin.h"
#include <stdio.h>
COutputPin::COutputPin()
:m_pFilter(NULL)
,m_index(PIN_INVALID_INDEX)
,m_id(PIN_INVALID_INDEX)
,m_pTag(NULL)
,m_ref(0)
,m_cmd(0)
,m_start(MEDIA_FRAME_NONE_TIMESTAMP)
,m_clock(MEDIA_FRAME_NONE_TIMESTAMP)
,m_segment(0)
,m_start_segment(MEDIA_FRAME_NONE_TIMESTAMP)
,m_flag(0)
,m_it(m_connections.end())
#ifdef PIN_OUTPUT
,m_dump(NULL)
#endif
{
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
    else
        m_index = 0;
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

STDMETHODIMP_(IMediaType*) COutputPin::GetMediaType()
{
    return m_spMT;
}

STDMETHODIMP COutputPin::SetMediaType(IMediaType* pMT)
{
    if(NULL != pMT)
    {
        dom_ptr<IProfile> spProfile;
        JCHK(spProfile.QueryFrom(pMT),E_FAIL);

        IProfile::val* pVal = spProfile->Read("slice");
        if(NULL != pVal)
        {
            char* pUnit = NULL;
            double val = strtod((const char*)pVal->value, &pUnit);
            if(0.0 < val)
            {
                if(*pUnit == 's' || *pUnit == 'S')
                    m_segment = int64_t(val * 10000000);
                else
                    m_segment = int64_t(val * 10000);
            }
        }
        spProfile->Erase("slice");
    }
    m_spMT = pMT;
    return S_OK;
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

STDMETHODIMP_(void) COutputPin::SetID(uint32_t id)
{
    m_id = id;
}

STDMETHODIMP_(uint32_t) COutputPin::GetID()
{
    return m_id;
}

STDMETHODIMP COutputPin::Send(uint32_t cmd,bool down,bool first)
{
    HRESULT hr = S_OK;
    if(true == down)
    {
        bool is_ok = true;
        for(ConnectionIt it = m_connections.begin() ; it !=  m_connections.end() ; ++it)
        {
            dom_ptr<IInputPin>& spPin = *it;
            JIF(spPin->Send(cmd,down,first));
            if(S_OK != hr)
                is_ok = false;
        }
        hr = true == is_ok ? S_OK : S_FALSE;
    }
    else
    {
        bool is_first = true;
        bool is_last = true;
        for(ConnectionIt it = m_connections.begin() ; it !=  m_connections.end(); ++it)
        {
            dom_ptr<IInputPin>& spPin = *it;
            if(cmd == spPin->Recv())
                is_first = false;
            else
                is_last = false;
        }

        if(true == is_last)
        {
            for(ConnectionIt it = m_connections.begin() ; it !=  m_connections.end(); ++it)
            {
                dom_ptr<IInputPin>& spPin = *it;
                spPin->Set(Filter_CMD_None);
            }
        }

        if((true == is_first && true == first) || (true == is_last && false == first))
        {

            is_first = true;
            is_last = true;

            uint32_t count = m_pFilter->GetOutputPinCount();
            for(uint32_t i=0 ; i<count ; ++i)
            {
                if(i != m_index)
                {
                    IOutputPin* pPin = m_pFilter->GetOutputPin(i);
                    if(NULL != pPin->GetConnection())
                    {
                        if(cmd == pPin->Recv())
                            is_first = false;
                        else
                            is_last = false;
                    }
                }
            }

            if(is_last)
            {
                for(uint32_t i=0 ; i<count ; ++i)
                {
                    IOutputPin* pPin = m_pFilter->GetOutputPin(i);
                    if(NULL != pPin->GetConnection())
                    {
                        pPin->Set(Filter_CMD_None);
                    }
                }
            }
            else
                m_cmd = cmd;

            if((true == is_first && true == first) || (true == is_last && false == first))
            {
                JIF(m_pFilter->Notify(cmd));
                if(S_OK == hr)
                {
                    count = m_pFilter->GetInputPinCount();
                    for(uint32_t i=0 ; i<count ; ++i)
                    {
                        IInputPin* pPin = m_pFilter->GetInputPin(i);
                        if(NULL != pPin->GetConnection())
                        {
                            JIF(pPin->Send(cmd,down,first));
                        }
                    }
                }
            }
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) COutputPin::Recv()
{
    return m_cmd;
}

STDMETHODIMP_(void) COutputPin::Set(uint32_t cmd)
{
    m_cmd = cmd;
}

STDMETHODIMP COutputPin::Write(IMediaFrame* pFrame)
{
    CLocker locker(m_locker);
    if(m_ep != NULL)
    {
        if(NULL != pFrame)
            m_frames.push_back(pFrame);
        return S_OK;
    }
    else
    {
        FrameIt it;
        while(m_frames.end() != (it = m_frames.begin()))
        {
            Render(*it);
            m_frames.erase(it);
        }
        return Render(pFrame);
    }
}

STDMETHODIMP_(void) COutputPin::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) COutputPin::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(void) COutputPin::SetObj(Interface* pObj)
{
    m_obj = pObj;
}

STDMETHODIMP_(Interface*) COutputPin::GetObj()
{
    return m_obj;
}

STDMETHODIMP COutputPin::SetFlag(uint8_t flag)
{
    JCHK(0 != flag,E_INVALIDARG);
    m_flag = flag;
    return S_OK;
}

//IOutputPin
STDMETHODIMP COutputPin::Connect(IInputPin* pPin,IMediaType* pMT)
{
    HRESULT hr;
    JCHK(NULL != pPin,E_INVALIDARG);
    CLocker locker(m_locker);
    if(pMT == NULL)
    {
        if(m_spMT != NULL)
            pMT = m_spMT.p;
        else
            pMT = pPin->GetMediaType();

        JCHK(NULL != pMT,E_INVALIDARG);
    }

    JIF(m_pFilter->OnConnect(this,pPin,pMT));
    JIF(pPin->Connect(this,pMT));
    JIF(SetMediaType(pMT));

    return hr;
}

STDMETHODIMP_(void) COutputPin::Disconnect(IInputPin* pPin)
{
    CLocker locker(m_locker);
    if(NULL != pPin)
    {
        pPin->Disconnect();
        m_pFilter->OnConnect(this,pPin,NULL);
    }
    else
    {
        ConnectionIt it;
        while(m_connections.end() != (it = m_connections.begin()))
        {
            dom_ptr<IInputPin> pin = *it;
            pin->Disconnect();
            m_pFilter->OnConnect(this,pin,NULL);
        }
    }
    if(true == m_connections.empty())
    {
        m_pFilter->OnConnect(this,NULL,NULL);
#ifdef PIN_OUTPUT
        if(NULL != m_dump)
        {
            fclose(m_dump);
            m_dump = NULL;
        }
#endif
    }
}

STDMETHODIMP_(IInputPin*) COutputPin::GetConnection()
{
    return true == m_connections.empty() ? NULL : m_connections.front().p;
}

STDMETHODIMP COutputPin::AllocFrame(IMediaFrame** ppFrame)
{
    return m_allocate->Alloc(ppFrame);
}

STDMETHODIMP COutputPin::SetClock(bool enable)
{
    HRESULT hr = S_OK;
    if(true == enable)
    {
        if(m_ep == NULL)
        {
            dom_ptr<IEpoll> spEpoll;
            JCHK(spEpoll.QueryFrom(g_pSite->GetObj()),false);
            JCHK(S_OK == spEpoll->CreatePoint(this,&m_ep),false);
            JIF(m_ep->SetTimer(0,100,false));
        }
    }
    else if(m_ep != NULL)
        m_ep = NULL;
    return hr;
}

STDMETHODIMP COutputPin::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(ET_Epoll_Timer == type)
    {
        if(0 == param1)
        {
            CLocker locker(m_locker);

            int64_t clock = *(int64_t*)param3;

            FrameIt it;
            while(m_frames.end() != (it = m_frames.begin()))
            {
                dom_ptr<IMediaFrame>& spFrame = *it;
                if(MEDIA_FRAME_NONE_TIMESTAMP == m_start || 0 != (spFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
                {
                    m_start = spFrame->info.dts;
                    m_clock = clock;
                }

                int64_t cur = (spFrame->info.dts - m_start)/10000;
                int64_t now = clock - m_clock;
                if(cur <= now)
                {
                    LOG(0,"%s stream ouput:%ld frame flag:%u",m_spMT->GetMajorName(),spFrame->info.dts,spFrame->info.flag);
                    Render(spFrame);
                    m_frames.erase(it);
                }
                else
                    break;
            }
        }
    }
    return S_OK;
}

ConnectionIt COutputPin::Connect(IInputPin* pPin)
{
    CLocker locker(m_locker);
    return m_connections.insert(m_connections.end(),pPin);
}

void COutputPin::Disconnect(ConnectionIt it)
{
    CLocker locker(m_locker);
    if(m_it == it)
        m_it = m_connections.erase(it);
    else
        m_connections.erase(it);
}

IInputPin* COutputPin::Next(ConnectionIt it)
{
    CLocker locker(m_locker);
    return ++it == m_connections.end() ? NULL : it->p;
}

HRESULT COutputPin::Render(IMediaFrame* pFrame)
{
    if(NULL != pFrame)
    {
        if(0 != m_flag)
        {
            if(0 == (m_flag & MEDIA_FRAME_FLAG_SYNCPOINT) || 0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
            {
                if(pFrame->info.flag != m_flag)
                    pFrame->info.flag |= m_flag;
                m_flag = 0;
            }
        }

        if(0 < m_segment)
        {
            if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
                m_start_segment = MEDIA_FRAME_NONE_TIMESTAMP;

            bool is_key = 0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT);

            if(MEDIA_FRAME_NONE_TIMESTAMP == m_start_segment)
            {
                if(false == is_key)
                    return S_OK;
                m_start_segment = pFrame->info.dts;
                pFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                //printf("COutputPin::Render dts:%ld flag:%d\n",pFrame->info.dts,pFrame->info.flag);
            }
            else if(true == is_key && pFrame->info.dts - m_start_segment >= m_segment)
            {
                m_start_segment = pFrame->info.dts;
                pFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                //printf("COutputPin::Render dts:%ld flag:%d\n",pFrame->info.dts,pFrame->info.flag);
            }
            else
                pFrame->info.flag &= ~MEDIA_FRAME_FLAG_SEGMENT;
        }
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
    }
    HRESULT hr = E_EOF;
    m_it = m_connections.begin();
    while(m_it != m_connections.end())
    {
        ConnectionIt it = m_it;
        HRESULT result = (*m_it)->Write(pFrame);
        if(E_EOF != result)
            hr = result;
        if(m_it == it)
            ++m_it;
    }
    return hr;
}
