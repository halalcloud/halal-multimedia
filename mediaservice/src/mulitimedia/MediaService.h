#ifndef MEDIASERVICE_H
#define MEDIASERVICE_H
#include "stdafx.h"
#include "ApiSession.h"

class CMediaService : public IMediaService , public ICallback
{
    friend class CApiSession;

    typedef map< uint16_t,dom_ptr<IStreamListen> > ListenSet;
    typedef ListenSet::iterator ListenIt;
    typedef pair<ListenSet::key_type,ListenSet::mapped_type> ListenPair;

    typedef list< dom_ptr<IFilter> > SessionSet;
    typedef SessionSet::iterator SessionIt;

    typedef multimap< string,dom_ptr<IGraphBuilder> > GraphSet;
    typedef GraphSet::iterator GraphIt;
    typedef pair<GraphSet::key_type,GraphSet::mapped_type> GraphPair;

    struct PublishPoint
    {
        IGraphBuilder* pGB;
        IOutputPin* pPin;
    };
    typedef map< string,PublishPoint > PublishSet;
    typedef PublishSet::iterator PublishIt;
    typedef pair<PublishSet::key_type,PublishSet::mapped_type> PublishPair;

    //typedef map< string,shared_ptr<CMediaClient> > ClientSet;
public:
    DOM_DECLARE(CMediaService)
    //IMediaService
    STDMETHODIMP StartUp(const char* pConfig);
    STDMETHODIMP Shutdown();
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
protected:
    string FindCfg(const char* stream,target_type target,method_type method);
    HRESULT Build(target_type target,method_type method,IFilter* filter);
    HRESULT Append(IFilter* pFilter);
    HRESULT Append(target_type target,IGraphBuilder* pGB);
protected:
    dom_ptr<SET(ListenSet)>  m_listeners;
    dom_ptr<SET(SessionSet)> m_sessions;
    dom_ptr<SET(GraphSet)>   m_graphs[target_nb];
    dom_ptr<SET(PublishSet)> m_publishs;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
    string m_root;
    CLocker m_locker_session;
    CLocker m_locker_graph;
    CLocker m_locker_point;
};

#endif // MEDIASERVICE_H
