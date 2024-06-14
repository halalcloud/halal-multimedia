#include <fstream>
#include <sstream>
#include "MediaService.h"

CMediaService::CMediaService()
{
    //ctor
}

bool CMediaService::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(NULL != (m_listeners = SET(ListenSet)::Create(NULL,false,(ICallback*)this)),false);
    JCHK(NULL != (m_sessions = SET(SessionSet)::Create(NULL,false,(ICallback*)this)),false);
    JCHK(NULL != (m_publishs = SET(PublishSet)::Create(NULL,false,(ICallback*)this)),false);
    for(uint32_t i=0 ; i< target_nb ; ++i)
    {
        JCHK(NULL != (m_graphs[i] = SET(GraphSet)::Create(NULL,false,(ICallback*)this)),false);
    }
    JCHK(m_ep.Create(CLSID_CEventPoint,(IMediaService*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IMediaService*)this,true),false);
    return true;
}

bool CMediaService::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Shutdown();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CMediaService)
DOM_QUERY_IMPLEMENT(IMediaService)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CMediaService::StartUp(const char* pConfig)
{
    HRESULT hr;
    JCHK(NULL != pConfig,E_INVALIDARG);

    ifstream ifs(pConfig);
    string json((istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    m_root.clear();
    if(false == json.empty())
    {
        Parser parser;
        Dynamic::Var var = parser.parse(json);
        Object::Ptr config = var.extract<Object::Ptr>();
        if(false == config.isNull())
        {
            Convert(g_pSite->GetProfile(),config);
            IProfile::val* pVal;
            if(NULL != (pVal = g_pSite->GetProfile()->Read("root")))
            {
                if(true == STR_CMP(pVal->type,typeid(char*).name()) || true == STR_CMP(pVal->type,typeid(const char*).name()))
                {
                    m_root = (char*)pVal->value;
                    if(m_root.back() == '/')
                        m_root.erase(m_root.end()-1);
                }
            }
        }
    }

    ClassSet classes;
    JIF(QueryFilter(classes,FT_Session));

    for(ClassIt it=classes.begin() ; it!=classes.end() ; ++it)
    {
        CREATE_LISTENER* func;
        if(NULL != (func = (CREATE_LISTENER*)it->second->ext2))
        {
            uint16_t port = 0;
            dom_ptr<IStreamListen> spListen;
            if(IS_OK(func(&spListen,NULL,&port)))
            {
                pair<ListenIt,bool> result = m_listeners->m_set.insert(ListenPair(port,spListen));
                if(true == result.second)
                {
                    dom_ptr<IEventPoint> spEP;
                    JCHK(spListen.Query(&spEP),E_FAIL);

                    dom_ptr<IIt> spIt;
                    JIF(m_listeners->CreateIt(&spIt,&result.first));
                    JIF(spEP->NotifySet(spIt));
                    if(IS_FAIL(spListen->Startup(it->second,port)))
                    {
                        LOG(1,"protocol:[%s:%d] listener srartup fail",it->second->desc,port);
                    }
                    else
                    {
                        LOG(0,"protocol:[%s:%d] listener srartup success",it->second->desc,port);
                    }
                }
                else
                {
                    LOG(1,"protocol:[%s:%d] create listener fail,port exist",it->second->desc,port);
                }
            }
        }
    }
    return hr;
}

STDMETHODIMP CMediaService::Shutdown()
{
    HRESULT hr = S_OK;

    for(uint32_t i=0 ; i<target_nb ; ++i)
    {
        for(GraphIt it = m_graphs[i]->m_set.begin() ; it != m_graphs[i]->m_set.end() ; ++it)
        {
            dom_ptr<IEventPoint> spEP;
            JCHK(it->second.Query(&spEP),E_FAIL);
            JIF(spEP->NotifySet(NULL));
        }
        m_graphs[i]->Clear();
    }
    for(SessionIt it = m_sessions->m_set.begin() ; it != m_sessions->m_set.end() ; ++it)
    {
        dom_ptr<IFilter>& spFilter = *it;
        dom_ptr<IEventPoint> spEP;
        JCHK(spFilter.Query(&spEP),E_FAIL);
        JIF(spEP->NotifySet(NULL));
    }
    m_sessions->Clear();
    for(ListenIt it = m_listeners->m_set.begin() ; it != m_listeners->m_set.end() ; ++it)
    {
        dom_ptr<IEventPoint> spEP;
        JCHK(it->second.Query(&spEP),E_FAIL);
        JIF(spEP->NotifySet(NULL));
    }
    m_listeners->Clear();
    m_publishs->Clear();
    return hr;
}

STDMETHODIMP CMediaService::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    HRESULT hr;
    switch(type)
    {
        case ET_EOF:
        {
            JCHK(NULL != it,E_FAIL);
            it->Erase();
            return S_OK;
//            if(&m_sessions == it->Tag())
//            {
//                dom_ptr<IFilter> spFilter;
//                JCHK(spFilter.QueryFrom(source),E_FAIL);
//                LOG(0,"%s[%p]:[%s] error return:%d",source->Class().name,source,spFilter->GetName(),param1);
//            }
//            else if(&m_graphs == it->Tag())
//            {
//                GraphIt graph_it;
//                dom_ptr<IGraphBuilder> spGB;
//                JCHK(spGB.QueryFrom(source),E_FAIL);
//                it->Get(&graph_it);
//                LOG(0,"%s[%p]:[%s] error return:%d",source->Class().name,source,graph_it->first.c_str(),param1);
//            }
        }
        break;
        case ET_Listen_Accept:
        {
            IFilter* pFilter;
            JCHK(NULL != (pFilter = (IFilter*)param2),E_FAIL);
            JIF(Append(pFilter));

            dom_ptr<IStreamListen> spListen;
            JCHK(spListen.QueryFrom(it->Get()),E_FAIL);
            LOG(0,"%s[%p] accept %s[%p]",spListen->Class().name,spListen.p,pFilter->Class().name,pFilter);

            JIF(pFilter->Notify(IFilter::C_Accept));

        }
        break;
        case ET_Session_Push:
        {
            dom_ptr<IFilter> spFilter;
            JCHK(spFilter.QueryFrom(it->Get()),E_FAIL);
            const char* pName = spFilter->GetName();

            IOutputPin* pPinOut;
            JCHK(pPinOut = (IOutputPin*)param2,E_FAIL);

            IMediaType* pMt;
            JCHK(pMt = pPinOut->GetMediaType(),E_FAIL);

            if(MST_NONE != pMt->GetSub()) // media
            {
                uint32_t flag = spFilter->GetFlag();
                if(0 != (flag & IFilter::FLAG_LIVE))
                {
                    GraphIt it_graph = m_graphs[target_live]->m_set.find(pName);
                    if(m_graphs[target_live]->m_set.end() != it_graph)
                        return it_graph->second->Set(spFilter);
                    return Build(target_live,method_push,spFilter);
                }
                else
                    return Build(target_vod,method_push,spFilter);
            }
            else //file
                return Build(target_file,method_push,spFilter);
        }
        break;
        case ET_Session_Pull:
        {
            IInputPin* pPinIn;
            JCHK(pPinIn = (IInputPin*)param2,E_FAIL);

            IMediaType* pMt;
            JCHK(pMt = pPinIn->GetMediaType(),E_FAIL);

            dom_ptr<IFilter> spFilter;
            JCHK(spFilter.QueryFrom(it->Get()),E_FAIL);
            if(MST_NONE != pMt->GetSub())
            {
                uint32_t flag = spFilter->GetFlag();
                if(0 != (flag & IFilter::FLAG_LIVE))
                {
                    CLocker locker(m_locker_graph);
                    const char* pName = spFilter->GetName();
                    PublishIt itPutlish = m_publishs->m_set.find(pName);
                    if(itPutlish != m_publishs->m_set.end())
                    {
                        PublishPoint& point = itPutlish->second;
                        LOG(0,"%s[%p] find graph:[%s] publish point:[%s]",
                            spFilter->Class().name,spFilter.p,point.pGB->GetName(),itPutlish->first.c_str());
                        return point.pGB->PublishPointAddClient(point.pPin,(IInputPin*)param2);
                    }
                    else
                    {
//                        hr = Build(target_live,method_pull,spFilter);
//                        if(S_OK <= hr)
//                            return hr;
                    }
                }
                return Build(target_vod,method_pull,spFilter);
            }
            else
            {
                return Build(target_file,method_pull,spFilter);
            }
        }
        break;
        case ET_Publish_Add:
        {
            CLocker locker(m_locker_graph);

            dom_ptr<IGraphBuilder> spGB;
            JCHK(spGB.QueryFrom(it->Get()),E_FAIL);

            PublishPoint point;
            JCHK(point.pGB = spGB.p,E_FAIL);
            JCHK(point.pPin = (IOutputPin*)param2,E_FAIL);
            const char* pName = point.pPin->GetFilter()->GetName();
            pair<PublishIt,bool> result = m_publishs->m_set.insert(PublishPair(pName,point));
            JCHK(true == result.second,E_FAIL);

            dom_ptr<IIt> spIt;
            JIF(m_publishs->CreateIt(&spIt,&result.first));
            point.pPin->SetObj(spIt);
            LOG(0,"graph:[%s] publish point:[%s] add",point.pGB->GetName(),pName);
        }
        break;
        case ET_Publish_Del:
        {
            CLocker locker(m_locker_graph);
            IOutputPin* pPin;
            JCHK(pPin = (IOutputPin*)param2,E_FAIL);

            IIt* pIt;
            JCHK(pIt = (IIt*)pPin->GetObj(),E_FAIL);

            PublishIt itPublish;
            pIt->Get(&itPublish);
            PublishPoint& point = itPublish->second;
            LOG(0,"graph:[%s] publish point:[%s] del",point.pGB->GetName(),itPublish->first.c_str());
            pIt->Erase();
            pPin->SetObj(NULL);
        }
        break;
        case ET_Api_Push:
        {
            char* method;
            IFilter* pFilter;
            JCHK(NULL != (pFilter = (IFilter*)param2),E_INVALIDARG);
            JCHK(NULL != (method = (char*)param3),E_INVALIDARG);
            if(0 == strcmp(method,"live_pull"))
                hr = Build(target_live,method_pull,pFilter);
            else
                hr = E_INVALIDARG;
        }
        break;
        default:
            hr = m_ep->Notify(type,param1,param2,param3);
    }
    return hr;
}

const char DEFAULT_FILE_NAME[] = "*";

string CMediaService::FindCfg(const char* stream,target_type target,method_type method)
{
    CUrl url;
    JCHK(S_OK == url.SetStreamID(NULL,stream),"");

    string path;
    string format = url.m_format;

    if(false == format.empty())
        url.m_format += '.';
    url.m_format += TARGET_NAME[target];
    url.m_format += '.';
    url.m_format += METHOD_NAME[method];

    path = url.GetPath(m_root.c_str());
    if(0 == access(path.c_str(),R_OK))
        return  path;
    if(false == format.empty())
    {
        url.m_format = TARGET_NAME[target];
        url.m_format += '.';
        url.m_format += METHOD_NAME[method];
        path = url.GetPath(m_root.c_str());
        if(0 == access(path.c_str(),R_OK))
            return  path;
    }
    path = '/';
    path += url.m_host;
    path += url.m_path;
    url.m_path = path;
    url.m_host.clear();
    url.m_file = DEFAULT_FILE_NAME;
    while(true)
    {
        url.m_format = format;
        if(false == format.empty())
            url.m_format += '.';
        url.m_format += TARGET_NAME[target];
        url.m_format += '.';
        url.m_format += METHOD_NAME[method];

        path = url.GetPath(m_root.c_str());
        if(0 == access(path.c_str(),R_OK))
            return  path;

        if(false == format.empty())
        {
            url.m_format = TARGET_NAME[target];
            url.m_format += '.';
            url.m_format += METHOD_NAME[method];
            path = url.GetPath(m_root.c_str());
            if(0 == access(path.c_str(),R_OK))
                return  path;
        }
        url.m_path.erase(url.m_path.end()-1);
        if(true == url.m_path.empty())
            break;
        url.m_path.erase(url.m_path.find_last_of('/')+1);

    }
    path.clear();
    return path;
}

HRESULT CMediaService::Build(target_type target,method_type method,IFilter* filter)
{
    JCHK(target_nb > target,E_INVALIDARG);
    JCHK(method_nb > method,E_INVALIDARG);
    JCHK(NULL != filter,E_INVALIDARG);

    HRESULT hr;
    const char* stream;
    JCHK(stream = filter->GetName(),E_INVALIDARG);
    string cfg = FindCfg(stream,target,method);
    if(true == cfg.empty())
    {
        LOG(0,"%s[%p]:[%s] can not find match target:[%s] method:[%s] config file",filter->Class().name,filter,stream,TARGET_NAME[target],METHOD_NAME[method]);
        return E_INVALIDARG;
    }
    else
    {
        LOG(0,"%s[%p]:[%s] find match target:[%s] method:[%s] config file:[%s]",filter->Class().name,filter,stream,TARGET_NAME[target],METHOD_NAME[method],cfg.c_str());
    }

    ifstream ifs(cfg);
    string json((istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    dom_ptr<IGraphBuilder> spGB;
    JCHK(spGB.Create(CLSID_CGraphBuilder),E_FAIL);

    JIF(spGB->Set(filter));
    JIF(Append(target,spGB));
    hr = spGB->Load(json.c_str());
    if(S_OK > hr)
    {
        LOG(1,"%s[%p]:[%s] load:[%s] fail",filter->Class().name,filter,filter->GetName(),cfg.c_str());
    }
    else
    {
        LOG(1,"%s[%p]:[%s] load:[%s] success create graph:[%s]",filter->Class().name,filter,filter->GetName(),cfg.c_str(),spGB->GetName());
    }
    return hr;
}

HRESULT CMediaService::Append(IFilter* pFilter)
{
    JCHK(NULL != pFilter,E_INVALIDARG);

    CLocker locker(m_locker_session);

    HRESULT hr;

    SessionIt it = m_sessions->m_set.insert(m_sessions->m_set.end(),pFilter);

    dom_ptr<IIt> spIt;
    JIF(m_sessions->CreateIt(&spIt,&it));

    dom_ptr<IEventPoint> spEP;
    JCHK(spEP.QueryFrom(pFilter),E_FAIL);
    JIF(spEP->NotifySet(spIt));

    return hr;
}

HRESULT CMediaService::Append(target_type target,IGraphBuilder* pGB)
{
    JCHK(target_nb > target,E_INVALIDARG);
    JCHK(NULL != pGB,E_INVALIDARG);

    HRESULT hr;

    CLocker locker(m_locker_graph);

    const char* pName = pGB->GetName();
    GraphIt it = m_graphs[target]->m_set.insert(GraphPair(pName,pGB));

    dom_ptr<IIt> spIt;
    JIF(m_graphs[target]->CreateIt(&spIt,&it));

    dom_ptr<IEventPoint> spEP;
    JCHK(spEP.QueryFrom(pGB),E_FAIL);
    JIF(spEP->NotifySet(spIt));
    spIt = NULL;
    return hr;
}
