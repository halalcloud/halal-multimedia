#include "FilterGraph.h"

CFilterGraph::CFilterGraph()
:m_callback(NULL)
{
    m_status.isLive = false;
    m_status.status = IFilter::S_Stop;
    m_status.clockStart = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.clockTime = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.timeStart = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.timeInput = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.timeOutput = MEDIA_FRAME_NONE_TIMESTAMP;
}

bool CFilterGraph::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(NULL != (m_callback = (IEventCallback*)pOuter),false);
    JCHK(NULL != (m_sources = SET(FilterSet)::Create(NULL,false,(ICallback*)this)),false);
    JCHK(NULL != (m_renders = SET(FilterSet)::Create(NULL,false,(ICallback*)this)),false);
    JCHK(NULL != (m_filters = SET(FilterSet)::Create(NULL,false,(ICallback*)this)),false);
    return true;
}

bool CFilterGraph::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFilterGraph)
DOM_QUERY_IMPLEMENT(IFilterGraph)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CFilterGraph::Create(FilterType type,const char* pUrl,IFilter** ppFilter,const char* pFormat,const char* pName)
{
    JCHK(FT_Source == type || FT_Render == type,E_INVALIDARG);
    JCHK(NULL != pUrl,E_INVALIDARG);
    JCHK(NULL != ppFilter,E_INVALIDARG);

    HRESULT hr;
    CUrl url;
    JIF(url.Set(pUrl));

    const char* pProtocol = url.m_protocol.empty() ? NULL : url.m_protocol.c_str();

    if(NULL != pFormat && 0 < strlen(pFormat))
        url.m_format = pFormat;
    else
        pFormat = url.m_format.empty() ? NULL : url.m_format.c_str();
    if(NULL != pName && 0 == strlen(pName))
        pName = NULL;

    ClassSet formats;
    QueryFilter(formats,type,pName,pProtocol,pFormat,NULL,NULL);
    for(ClassIt it = formats.begin() ; it != formats.end() ; ++it)
    {
        dom_ptr<IFilter> spFilter;
        spFilter.p = static_cast<IFilter*>(it->second->func(IID(IFilter),NULL,false,NULL));
        if(spFilter != NULL)
        {
            JIF(spFilter->SetName(url.Get()));
            JIF(Append(type,spFilter));
            return spFilter.CopyTo(ppFilter);
        }
    }

    JCHK2(false,E_INVALIDARG,"url[%s] can not find support name:[%s] filter",pUrl,pName);

    return E_INVALIDARG;
}

STDMETHODIMP CFilterGraph::ConnectPin(IOutputPin* pPinOut,IInputPin* pPinIn,IMediaType* pMT)
{
    return ConnectPin(pPinOut,pPinIn,pMT,true);
}

STDMETHODIMP CFilterGraph::ConnectPin(IOutputPin* pPin,IFilter** ppFilter,const char* pName)
{
    JCHK(NULL != pPin,E_INVALIDARG);

    HRESULT hr = E_FAIL;

    ClassSet filters;
    QueryFilter(filters,FT_Transform,pName,NULL,NULL,pPin->GetMediaType(),NULL);

    for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
    {
        dom_ptr<IFilter> spFilter;
        if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->func(IID(IFilter),NULL,false,NULL))))
        {
            JIF(pPin->Connect(spFilter->GetInputPin(0),pPin->GetMediaType()));
            JIF(Append(spFilter));
            return NULL == ppFilter ? S_OK : spFilter.CopyTo(ppFilter);
        }
    }
    return hr;
}

STDMETHODIMP CFilterGraph::ConnectPin(IInputPin* pPin,IFilter** ppFilter,const char* pName)
{
    JCHK(NULL != pPin,E_INVALIDARG);

    HRESULT hr = E_FAIL;

    ClassSet filters;
    QueryFilter(filters,FT_Transform,pName,NULL,NULL,NULL,pPin->GetMediaType());

    for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
    {
        dom_ptr<IFilter> spFilter;
        if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->func(IID(IFilter),NULL,false,NULL))))
        {
            JIF(spFilter->GetOutputPin(0)->Connect(pPin,pPin->GetMediaType()));
            JIF(Append(spFilter));
            return NULL == ppFilter ? S_OK : spFilter.CopyTo(ppFilter);
        }
    }
    return hr;
}

STDMETHODIMP CFilterGraph::Append(IFilter* pFilter)
{
    return Append(pFilter->GetType(),pFilter);
}

STDMETHODIMP CFilterGraph::Enum(FilterType type,IIt** ppIt)
{
    if(FT_Source == type)
        return m_sources->CreateIt(ppIt);
    else if(FT_Render == type)
        return m_renders->CreateIt(ppIt);
    else if(FT_Transform == type)
        return m_filters->CreateIt(ppIt);
    else
        return E_INVALIDARG;
}

STDMETHODIMP_(IFilter*) CFilterGraph::Get(IIt* pIt)
{
    JCHK(NULL != pIt,NULL);
    FilterIt it;
    pIt->Get(&it);
    return *it;
}

STDMETHODIMP_(uint32_t) CFilterGraph::GetCount(FilterType type)
{
    if(FT_Source == type)
        return m_sources->m_set.size();
    else if(FT_Transform == type)
        return m_filters->m_set.size();
    else if(FT_Render == type)
        return m_renders->m_set.size();
    else
        return 0;
}

STDMETHODIMP CFilterGraph::Notify(IFilter* pFilter,uint32_t cmd,bool first)
{
    HRESULT hr;
    if(true == first)
    {
        JIF(pFilter->Notify(cmd));
    }
    bool is_ok = true;
    uint32_t count = pFilter->GetOutputPinCount();
    for(uint32_t i=0 ; i<count ; ++i)
    {
        dom_ptr<IOutputPin> spPinOut;
        JCHK(spPinOut = pFilter->GetOutputPin(i),E_FAIL);
        JIF(spPinOut->Send(cmd,true,first));
        if(S_OK != hr)
            is_ok = false;
    }
    if(false == first)
    {
        if(true == is_ok)
        {
            JIF(pFilter->Notify(cmd));
        }
        else
            hr = S_FALSE;
    }
    else
        hr = true == is_ok ? S_OK : S_FALSE;
    return hr;
}

STDMETHODIMP_(IFilterGraph::Status&) CFilterGraph::GetStatus()
{
    return m_status;
}

STDMETHODIMP CFilterGraph::Clear()
{
    for(FilterIt it = m_sources->m_set.begin();it != m_sources->m_set.end() ; ++it)
    {
        dom_ptr<IFilter>& spFilter = *it;
        Notify(spFilter,IFilter::S_Stop,true);

        dom_ptr<IEventPoint> spEP;
        JCHK(spFilter.Query(&spEP),E_FAIL);
        spEP->NotifySet(NULL);
    }
    for(FilterIt it = m_filters->m_set.begin();it != m_filters->m_set.end() ; ++it)
    {
        dom_ptr<IFilter>& spFilter = *it;
        dom_ptr<IEventPoint> spEP;
        JCHK(spFilter.Query(&spEP),E_FAIL);
        spEP->NotifySet(NULL);
    }
    for(FilterIt it = m_renders->m_set.begin();it != m_renders->m_set.end() ; ++it)
    {
        dom_ptr<IFilter>& spFilter = *it;
        dom_ptr<IEventPoint> spEP;
        JCHK(spFilter.Query(&spEP),E_FAIL);
        spEP->NotifySet(NULL);
    }
    m_sources->Clear();
    m_filters->Clear();
    m_renders->Clear();
    return S_OK;
}

STDMETHODIMP CFilterGraph::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    return m_callback->OnEvent(source,it,type,param1,param2,param3);
//    else if(ET_Filter_Render == type && NULL != param2 && NULL != param3)
//    {
//        IInputPin* pPin = (IInputPin*)param2;
//        IMediaFrame* pFrame = (IMediaFrame*)param3;
//        int64_t time = pFrame->info.dts;
//        if(MEDIA_FRAME_NONE_TIMESTAMP != time)
//        {
//            m_status.clockTime = GetTickCount();
//            if(MEDIA_FRAME_NONE_TIMESTAMP == m_status.clockStart)
//            {
//                m_status.clockStart = m_status.clockTime;
//                m_status.timeStart = time;
//            }
//            else
//            {
//                if(m_status.clockTime < m_status.clockStart)
//                {
//                    m_status.clockStart = m_status.clockTime;
//                    m_status.timeStart = time;
//                }
//                else if(IFilter::FLAG_LIVE == (pPin->GetFilter()->GetFlag() & IFilter::FLAG_LIVE))
//                {
//                    if(false == m_status.isLive)
//                        m_status.isLive = true;
//                    int64_t clock = m_status.clockTime - m_status.clockStart;
//                    int64_t delta = time - m_status.timeStart - clock;
//                    if(0 < delta)
//                    {
//                        __useconds_t us = delta/10;
//                        usleep(us);
//                    }
//                    else if(0 > delta)
//                    {
//                        //printf("delay %ldms\n",delta/10000);
//                    }
//                }
//            }
//        }
//        if(time > m_status.timeOutput)
//            m_status.timeOutput = time;
//        return S_OK;
//    }
}

HRESULT CFilterGraph::ConnectPin(IOutputPin* pPinOut,IInputPin* pPinIn,IMediaType* pMT,bool isDrop)
{
    JCHK(NULL != pPinOut,E_FAIL);
    JCHK(NULL != pPinIn,E_FAIL);

    HRESULT hr;
    if(S_OK == (hr = pPinOut->Connect(pPinIn,pMT)))
    {
        return hr;
    }
    dom_ptr<IMediaType> spMtOut,spMtIn;
    if(NULL == pMT)
    {
        JCHK(spMtIn = pPinIn->GetMediaType(),E_INVALIDARG);
        JCHK(spMtOut = pPinOut->GetMediaType(),E_INVALIDARG);
    }
    else
    {
        if(true == isDrop)
        {
            JCHK(spMtOut = pPinOut->GetMediaType(),E_INVALIDARG);
            spMtIn = pMT;
        }
        else
        {
            JCHK(spMtIn = pPinIn->GetMediaType(),E_INVALIDARG);
            spMtOut = pMT;
        }
    }

    JCHK(spMtOut->GetMajor() == spMtIn->GetMajor(),E_INVALIDARG);

    ClassSet filters;
    QueryFilter(filters,FT_Transform,NULL,NULL,NULL,spMtOut,spMtIn);
    bool is_cmp_out = spMtOut->IsCompress();
    bool is_cmp_in = spMtIn->IsCompress();

    for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
    {
        dom_ptr<IFilter> spFilter;
        if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->func(IID(IFilter),NULL,false,NULL))))
        {
            if(true == is_cmp_out || false == is_cmp_in)
            {
                if(S_OK <= (hr = pPinOut->Connect(spFilter->GetInputPin(0),spMtOut)))
                {
                    dom_ptr<IOutputPin> spPinOut = spFilter->GetOutputPin(0);
                    if(spPinOut == NULL || S_OK <= (hr = spPinOut->Connect(pPinIn,spMtIn)) ||
                       S_OK <= (hr = ConnectPin(spPinOut,pPinIn,spMtIn,true)))
                    {
                        JIF(Append(spFilter));
                        return spPinOut == NULL ? S_FALSE : hr;
                    }
                }
            }
            else
            {
                if(S_OK <= (hr = spFilter->GetOutputPin(0)->Connect(pPinIn,spMtIn)))
                {
                    if(S_OK <= (hr = pPinOut->Connect(spFilter->GetInputPin(0),spMtOut)) ||
                       S_OK <= (hr = ConnectPin(pPinOut,spFilter->GetInputPin(0),spMtOut,false)))
                    {
                        return Append(spFilter);
                    }
                }
            }
        }
    }


    if(true == is_cmp_out || false == is_cmp_in)
    {
        QueryFilter(filters,FT_Transform,NULL,NULL,NULL,spMtOut,NULL);
        JCHK1(false == filters.empty(),E_FAIL,"can not find input:%s filter",spMtOut->GetSubName());
        for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
        {
            dom_ptr<IFilter> spFilter;
            if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->func(IID(IFilter),NULL,false,NULL))))
            {
                if(S_OK <= (hr = pPinOut->Connect(spFilter->GetInputPin(0),spMtOut)))
                {
                    dom_ptr<IOutputPin> spPinOut = spFilter->GetOutputPin(0);
                    if(spPinOut == NULL || S_OK <= (hr = ConnectPin(spFilter->GetOutputPin(0),pPinIn,spMtIn,true)))
                    {
                        JIF(Append(spFilter));
                        return spPinOut == NULL ? S_FALSE : hr;
                    }
                }
            }
        }
    }
    else
    {
        QueryFilter(filters,FT_Transform,NULL,NULL,NULL,NULL,spMtIn);
        JCHK1(false == filters.empty(),E_FAIL,"can not find output%s filter",spMtOut->GetSubName());
        for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
        {
            dom_ptr<IFilter> spFilter;
            if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->func(IID(IFilter),NULL,false,NULL))))
            {
                if(S_OK <= (hr = spFilter->GetOutputPin(0)->Connect(pPinIn,spMtIn)))
                {
                    if(S_OK <= (hr = ConnectPin(pPinOut,spFilter->GetInputPin(0),spMtOut,false)))
                    {
                        return Append(spFilter);
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT CFilterGraph::Append(FilterType type,IFilter* pFilter)
{
    JCHK(NULL != pFilter,E_INVALIDARG);

    dom_ptr<SET(FilterSet)> sets;
    if(FT_Source == type)
        sets = m_sources;
    else if(FT_Render == type)
        sets = m_renders;
    else if(FT_Transform == type)
        sets = m_filters;
    else
        return E_INVALIDARG;

    HRESULT hr;

    FilterIt it = sets->m_set.insert(sets->m_set.end(),pFilter);

    dom_ptr<IIt> spIt;
    JIF(sets->CreateIt(&spIt,&it));

    dom_ptr<IEventPoint> spEP;
    JCHK(spEP.QueryFrom(pFilter),E_FAIL);
    JIF(spEP->NotifySet(spIt));
    return hr;
}
