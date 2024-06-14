#include "FilterGraph.h"
#include "../src/Url.cpp"

CFilterGraph::CFilterGraph()
:m_thread(0)
,m_pEvent(NULL)
,m_pTag(NULL)
,m_isExit(false)
{
    m_status.isLive = false;
    m_status.clockStart = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.clockTime = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.timeStart = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.timeInput = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.timeOutput = MEDIA_FRAME_NONE_TIMESTAMP;
}

bool CFilterGraph::FinalConstruct(Interface* pOuter,void* pParam)
{
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
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CFilterGraph::LoadSource(const char* pUrl,IFilter** ppFilter,const char* pName)
{
    dom_ptr<IDemuxer> spSource;
    SourceIt it = m_sources.find(pUrl);
    if(it != m_sources.end())
        spSource = it->second;
    else
    {
        CUrl url(pUrl);
        ClassSet formats;
        GetFormat(formats,FILTER_TYPE_SOURCE,url.protocol,url.format,pName);

        for(ClassIt it = formats.begin() ; it != formats.end() ; ++it)
        {
            dom_ptr<IDemuxer> spDemuxer;
            spDemuxer.p = static_cast<IDemuxer*>(it->second->pFunc(IID(IDemuxer),NULL,(IFilterGraphEvent*)this));
            if(S_OK == spDemuxer->Load(pUrl))
            {
                m_sources.insert(SourcePair(pUrl,spDemuxer));
                spSource = spDemuxer;
                break;
            }
        }
        JCHK1(spSource != NULL,E_FAIL,"url[%s] can not find support demuxer",pUrl);
    }
    JCHK(spSource.Query(ppFilter),E_FAIL);
    return S_OK;
}

STDMETHODIMP CFilterGraph::LoadRender(const char* pUrl,IFilter** ppFilter,const char* pName)
{
    dom_ptr<IMuxer> spRender;
    RenderIt it = m_renders.find(pUrl);
    if(it != m_renders.end())
        spRender = it->second;
    else
    {
        CUrl url(pUrl);
        ClassSet formats;
        GetFormat(formats,FILTER_TYPE_RENDER,url.protocol,url.format,pName);

        for(ClassIt it = formats.begin() ; it != formats.end() ; ++it)
        {
            dom_ptr<IMuxer> spMuxer;
            spMuxer.p = static_cast<IMuxer*>(it->second->pFunc(IID(IMuxer),NULL,(IFilterGraphEvent*)this));
            if(S_OK == spMuxer->Load(pUrl))
            {
                m_renders.insert(RenderPair(pUrl,spMuxer));
                spRender = spMuxer;
                break;
            }
        }
        JCHK1(spRender != NULL,E_FAIL,"url[%s] can not find support muxer",pUrl);
    }
    JCHK(spRender.Query(ppFilter),E_FAIL);
    return S_OK;
}

STDMETHODIMP CFilterGraph::ConnectPin(IOutputPin* pPinOut,IInputPin* pPinIn,IMediaType* pMT)
{
    return ConnectPin(pPinOut,pPinIn,pMT,true);
}

STDMETHODIMP CFilterGraph::ConnectPin(IOutputPin* pPin,IMediaType* pMt,const char* pName,IFilter** ppFilter)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != ppFilter,E_INVALIDARG);

    HRESULT hr = S_OK;

    dom_ptr<IMediaType> spMtOut;
    if(NULL == pMt)
    {
        JCHK(spMtOut.Create(CLSID_CMediaType),E_FAIL);
        JIF(pPin->GetMediaType(spMtOut));
    }
    else
        spMtOut = pMt;

    bool not_ok;
    ClassSet filters;
    GetFilter(filters,spMtOut,NULL,pName);
    if(true == (not_ok = filters.empty()))
    {
        JCHK0(NULL != pMt,E_FAIL,"can not find valid filter to connect pin");
        GetFilter(filters,spMtOut,NULL,NULL);
        JCHK0(false == filters.empty(),E_FAIL,"can not find valid filter to connect pin");
    }

    for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
    {
        dom_ptr<IFilter> spFilter;
        if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->pFunc(IID(IFilter),NULL,(IFilterGraphEvent*)this))))
        {
            if(S_OK <= (hr = pPin->Connect(spFilter->GetInputPin(0),spMtOut)))
            {
                if(true == not_ok)
                {
                    hr = ConnectPin(spFilter->GetOutputPin(0),NULL,pName,ppFilter);
                    if(S_OK > hr)
                        continue;
                }
                char name[60];
                snprintf(name,60,"%s_0x%p",it->second->pName,spFilter.p);
                JIF(spFilter->SetName(name));
                m_filters.insert(FilterPair(name,spFilter));
                if(false == not_ok)
                {
                    JIF(spFilter.CopyTo(ppFilter));
                }
                break;
            }
        }
    }
    return hr;
}

STDMETHODIMP CFilterGraph::ConnectPin(IInputPin* pPin,IMediaType* pMt,const char* pName,IFilter** ppFilter)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != ppFilter,E_INVALIDARG);

    HRESULT hr = S_OK;

    dom_ptr<IMediaType> spMtIn;
    if(NULL == pMt)
    {
        JCHK(spMtIn.Create(CLSID_CMediaType),E_FAIL);
        JIF(pPin->GetMediaType(spMtIn));
    }
    else
        spMtIn = pMt;

    bool not_ok;
    ClassSet filters;
    GetFilter(filters,NULL,spMtIn,pName);
    if(true == (not_ok = filters.empty()))
    {
        JCHK0(NULL != pMt,E_FAIL,"can not find valid filter to connect pin");
        GetFilter(filters,NULL,spMtIn,NULL);
        JCHK0(false == filters.empty(),E_FAIL,"can not find valid filter to connect pin");
    }

    for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
    {
        dom_ptr<IFilter> spFilter;
        if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->pFunc(IID(IFilter),NULL,(IFilterGraphEvent*)this))))
        {
            if(S_OK <= (hr = spFilter->GetOutputPin(0)->Connect(pPin,spMtIn)))
            {
                if(true == not_ok)
                {
                    hr = ConnectPin(spFilter->GetInputPin(0),NULL,pName,ppFilter);
                    if(S_OK > hr)
                        continue;
                }
                char name[60];
                snprintf(name,60,"%s_0x%p",it->second->pName,spFilter.p);
                JIF(spFilter->SetName(name));
                m_filters.insert(FilterPair(name,spFilter));
                if(false == not_ok)
                {
                    JIF(spFilter.CopyTo(ppFilter));
                }
                break;
            }
        }
    }
    return hr;
}

STDMETHODIMP CFilterGraph::Remove(IFilter* pFilter)
{
    JCHK(NULL != pFilter,E_INVALIDARG);
    if(STR_CMP(pFilter->Class().pType,FILTER_TYPE_SOURCE))
    {
        SourceIt it = m_sources.find(pFilter->GetName());
        if(it != m_sources.end())
        {
            m_sources.erase(it);
            return S_OK;
        }
    }
    else if(STR_CMP(pFilter->Class().pType,FILTER_TYPE_RENDER))
    {
        RenderIt it = m_renders.find(pFilter->GetName());
        if(it != m_renders.end())
        {
            m_renders.erase(it);
            return S_OK;
        }
    }
    else if(STR_CMP(pFilter->Class().pType,FILTER_TYPE_TRANSFORM))
    {
        FilterIt it = m_filters.find(pFilter->GetName());
        if(it != m_filters.end())
        {
            m_filters.erase(it);
            return S_OK;
        }
    }
    return S_FALSE;
}

STDMETHODIMP CFilterGraph::Play(bool isWait)
{
    JCHK(0 == pthread_create(&m_thread, NULL, Process, (void*)this),E_FAIL);
    m_isExit = false;
    if(true == isWait)
    {
        pthread_join(m_thread,NULL);
        m_thread = 0;
    }
    return S_OK;
}

STDMETHODIMP CFilterGraph::Stop(bool isWait)
{
    if(0 == m_thread)
        return S_OK;

    m_isExit = true;
    if(true == isWait)
    {
        if(0 != m_thread)
        {
            LOG(0,"filter graph stop wait begin");
            pthread_join(m_thread,NULL);
            m_thread = 0;
        }
    }
    LOG(0,"filter graph stop wait:%d",isWait);
    return S_OK;
}

STDMETHODIMP_(bool) CFilterGraph::IsRunning()
{
    return 0 != m_thread;
}

STDMETHODIMP CFilterGraph::SetEvent(IFilterEvent* pEvent,void* pTag)
{
    m_pEvent = pEvent;
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(const filter_graph_status&) CFilterGraph::GetStatus()
{
    return m_status;
}

STDMETHODIMP CFilterGraph::Notify(IFilterEvent::EventType type,HRESULT hr,IFilter* pFilter,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    if(IFilterEvent::Process == type && STR_CMP(pFilter->Class().pType,FILTER_TYPE_RENDER) && NULL != pFrame)
    {
        int64_t time = pFrame->info.dts;
        if(MEDIA_FRAME_NONE_TIMESTAMP != time)
        {
            m_status.clockTime = GetTickCount();
            if(MEDIA_FRAME_NONE_TIMESTAMP == m_status.clockStart)
            {
                m_status.clockStart = m_status.clockTime;
                m_status.timeStart = time;
            }
            else
            {
                if(m_status.clockTime < m_status.clockStart)
                {
                    m_status.clockStart = m_status.clockTime;
                    m_status.timeStart = time;
                }
                else if(FILTER_FLAG_LIVE == (pFilter->GetFlag() & FILTER_FLAG_LIVE))
                {
                    if(false == m_status.isLive)
                        m_status.isLive = true;
                    int64_t clock = m_status.clockTime - m_status.clockStart;
                    int64_t delta = time - m_status.timeStart - clock;
                    if(0 < delta)
                    {
                        __useconds_t us = delta/10;
                        usleep(us);
                    }
                    else if(0 > delta)
                    {
                        //printf("delay %ldms\n",delta/10000);
                    }
                }
            }
        }
        if(time > m_status.timeOutput)
            m_status.timeOutput = time;
    }
    return NULL == m_pEvent ? S_OK : m_pEvent->OnEvent(type,hr,pFilter,pPinIn,pPinOut,pFrame,m_pTag);
}

STDMETHODIMP_(bool) CFilterGraph::IsExit()
{
    return m_isExit;
}

STDMETHODIMP CFilterGraph::Clear()
{
    HRESULT hr;
    JIF(Stop(true));
    m_filters.clear();
    m_sources.clear();
    m_renders.clear();
    return hr;
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
        JCHK(spMtIn.Create(CLSID_CMediaType),E_FAIL);
        JCHK(spMtOut.Create(CLSID_CMediaType),E_FAIL);
        JIF(pPinIn->GetMediaType(spMtIn));
        JIF(pPinOut->GetMediaType(spMtOut));
    }
    else
    {
        if(true == isDrop)
        {
            JCHK(spMtOut.Create(CLSID_CMediaType),E_FAIL);
            JIF(pPinOut->GetMediaType(spMtOut));
            spMtIn = pMT;
        }
        else
        {
            JCHK(spMtIn.Create(CLSID_CMediaType),E_FAIL);
            JIF(pPinIn->GetMediaType(spMtIn));
            spMtOut = pMT;
        }
    }

    JCHK(spMtOut->GetMajor() == spMtIn->GetMajor(),E_INVALIDARG);

    ClassSet filters;
    GetFilter(filters,spMtOut,spMtIn);
    bool is_cmp_out = spMtOut->IsCompress();
    bool is_cmp_in = spMtIn->IsCompress();

    for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
    {
        dom_ptr<IFilter> spFilter;
        if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->pFunc(IID(IFilter),NULL,(IFilterGraphEvent*)this))))
        {
            char name[60];
            snprintf(name,60,"%s_0x%p",it->second->pName,spFilter.p);
            if(true == is_cmp_out || false == is_cmp_in)
            {
                if(S_OK <= (hr = pPinOut->Connect(spFilter->GetInputPin(0),spMtOut)))
                {
                    if(S_OK <= (hr = spFilter->GetOutputPin(0)->Connect(pPinIn,spMtIn)) ||
                       S_OK <= (hr = ConnectPin(spFilter->GetOutputPin(0),pPinIn,spMtIn,true)))
                    {
                        JIF(spFilter->SetName(name));
                        m_filters.insert(FilterPair(name,spFilter));
                        return hr;
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
                        JIF(spFilter->SetName(name));
                        m_filters.insert(FilterPair(name,spFilter));
                        return hr;
                    }
                }
            }
        }
    }


    if(true == is_cmp_out || false == is_cmp_in)
    {
        GetFilter(filters,spMtOut,NULL);
        JCHK1(false == filters.empty(),E_FAIL,"can not find input:%s filter",spMtOut->GetSubName());
        for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
        {
            dom_ptr<IFilter> spFilter;
            if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->pFunc(IID(IFilter),NULL,(IFilterGraphEvent*)this))))
            {
                if(S_OK <= (hr = pPinOut->Connect(spFilter->GetInputPin(0),spMtOut)))
                {
                    if(S_OK <= (hr = ConnectPin(spFilter->GetOutputPin(0),pPinIn,spMtIn,true)))
                    {
                        char name[60];
                        snprintf(name,60,"%s_0x%p",it->second->pName,spFilter.p);
                        JIF(spFilter->SetName(name));
                        m_filters.insert(FilterPair(name,spFilter));
                        return hr;
                    }
                }
            }
        }
    }
    else
    {
        GetFilter(filters,NULL,spMtIn);
        JCHK1(false == filters.empty(),E_FAIL,"can not find output%s filter",spMtOut->GetSubName());
        for(ClassIt it = filters.begin() ; it != filters.end() ; ++it)
        {
            dom_ptr<IFilter> spFilter;
            if(NULL != (spFilter.p = static_cast<IFilter*>(it->second->pFunc(IID(IFilter),NULL,(IFilterGraphEvent*)this))))
            {
                if(S_OK <= (hr = spFilter->GetOutputPin(0)->Connect(pPinIn,spMtIn)))
                {
                    if(S_OK <= (hr = ConnectPin(pPinOut,spFilter->GetInputPin(0),spMtOut,false)))
                    {
                        char name[60];
                        snprintf(name,60,"%s_0x%p",it->second->pName,spFilter.p);
                        JIF(spFilter->SetName(name));
                        m_filters.insert(FilterPair(name,spFilter));
                        return hr;
                    }
                }
            }
        }
    }
    return hr;
}

void* CFilterGraph::Process(void* pParam)
{
    CFilterGraph* pThis = (CFilterGraph*)pParam;
    pThis->Process();
    return NULL;
}

HRESULT CFilterGraph::Process()
{
    HRESULT hr = S_STREAM_EOF;
    do
    {
        dom_ptr<IDemuxer> spSource;
        int64_t timeInput = MEDIA_FRAME_NONE_TIMESTAMP;
        int64_t timeMin = MEDIA_FRAME_NONE_TIMESTAMP;
        for(SourceIt it = m_sources.begin() ; it != m_sources.end() ; ++it)
        {
            dom_ptr<IDemuxer> spDemuxer;
            spDemuxer = it->second;
            if(false == spDemuxer->IsEOF())
            {
                int64_t time = spDemuxer->GetTime();
                if(spSource == NULL)
                {
                    timeMin = time;
                    timeInput = time;
                    spSource = spDemuxer;
                }
                else
                {
                    if(time < timeMin)
                    {
                        timeMin = time;
                        spSource = spDemuxer;
                    }
                    if(time > timeInput)
                    {
                        timeInput = time;
                    }
                }
            }
        }
        if(spSource == NULL)
            hr = S_STREAM_EOF;
        else
        {
            if(MEDIA_FRAME_NONE_TIMESTAMP != timeInput)
                m_status.timeInput = timeInput;
            hr = spSource->Process();
        }
        //hr = m_pEvent->OnEvent(IFilterEvent::Process,hr,NULL,NULL,NULL,NULL,m_pTag);
    }while(S_STREAM_EOF != hr && IS_OK(hr));
    LOG(0,"filter graph thread exit:%d return %d",m_isExit,hr);
    if(NULL != m_pEvent)
        hr = m_pEvent->OnEvent(IFilterEvent::End,hr,NULL,NULL,NULL,NULL,m_pTag);
    m_thread = 0;
    return hr;
}

void CFilterGraph::GetFormat(ClassSet& classes,const char* pType,const char* protocol,const char* format,const char* pName)
{
    classes.clear();
    const ClassInfo* pCI = NULL;
    while(NULL != (pCI = g_pSite->Enum(pCI,pType,pName)))
    {
        URL_SUPPORT_QUERY_FUNC* pFunc = (URL_SUPPORT_QUERY_FUNC*)pCI->pExt;
        if(NULL != pFunc)
        {
            HRESULT hr = pFunc(protocol,format);
            if(0 <= hr)
            {
                classes.insert(ClassPair(hr,pCI));
            }
        }
    }
}

void CFilterGraph::GetFilter(ClassSet& classes,IMediaType* pMtOut,IMediaType* pMtIn,const char* pName)
{
    classes.clear();
    const ClassInfo* pCI = NULL;
    while(NULL != (pCI = g_pSite->Enum(pCI,FILTER_TYPE_TRANSFORM,pName)))
    {
        FILTER_CHECK_MEDIATYPE_FUNC* pFunc = (FILTER_CHECK_MEDIATYPE_FUNC*)pCI->pExt;
        if(NULL != pFunc)
        {
            HRESULT hr = pFunc(pMtOut,pMtIn);
            if(0 <= hr)
            {
                classes.insert(ClassPair(hr,pCI));
            }
        }
    }
}
