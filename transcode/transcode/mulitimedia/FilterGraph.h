#ifndef FILTERGRAPH_H
#define FILTERGRAPH_H
#include <iostream>
#include <map>
#include <list>

#include "stdafx.h"

class CFilterGraph : public IFilterGraph ,public IFilterGraphEvent
{
    typedef multimap< HRESULT,const ClassInfo*,greater<HRESULT> > ClassSet;
    typedef ClassSet::iterator ClassIt;
    typedef pair<ClassSet::key_type,ClassSet::mapped_type> ClassPair;

    typedef map< string,dom_ptr<IFilter> > FilterSet;
    typedef FilterSet::iterator FilterIt;
    typedef pair<FilterSet::key_type,FilterSet::mapped_type> FilterPair;

    typedef map< string,dom_ptr<IDemuxer> > SourceSet;
    typedef SourceSet::iterator SourceIt;
    typedef pair<SourceSet::key_type,SourceSet::mapped_type> SourcePair;

    typedef map< string,dom_ptr<IMuxer> > RenderSet;
    typedef RenderSet::iterator RenderIt;
    typedef pair<RenderSet::key_type,RenderSet::mapped_type> RenderPair;

public:
    DOM_DECLARE(CFilterGraph)
    STDMETHODIMP LoadSource(const char* pUrl,IFilter** ppFilter,const char* pName);
    STDMETHODIMP LoadRender(const char* pUrl,IFilter** ppFilter,const char* pName);
    STDMETHODIMP ConnectPin(IOutputPin* pPinOut,IInputPin* pPinIn,IMediaType* pMT);
    STDMETHODIMP ConnectPin(IOutputPin* pPin,IMediaType* pMt,const char* pName,IFilter** ppFilter);
    STDMETHODIMP ConnectPin(IInputPin* pPin,IMediaType* pMt,const char* pName,IFilter** ppFilter);
    STDMETHODIMP Remove(IFilter* pFilter);
    STDMETHODIMP Play(bool isWait = false);
    STDMETHODIMP Stop(bool isWait = true);
    STDMETHODIMP_(bool) IsRunning();
    STDMETHODIMP SetEvent(IFilterEvent* pEvent,void* pTag = NULL);
    STDMETHODIMP_(const filter_graph_status&) GetStatus();
    STDMETHODIMP Notify(IFilterEvent::EventType type,HRESULT hr,IFilter* pFilter,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame);
    STDMETHODIMP_(bool) IsExit();
    STDMETHODIMP Clear();
protected:
    HRESULT ConnectPin(IOutputPin* pPinOut,IInputPin* pPinIn,IMediaType* pMT,bool isDrop);
    HRESULT Process();
    static void* Process(void* pParam);
    static void GetFormat(ClassSet& classes,const char* pType,const char* protocol,const char* format,const char* pName);
    static void GetFilter(ClassSet& classes,IMediaType* pMtOut,IMediaType* pMtIn,const char* pName = NULL);
protected:
    SourceSet m_sources;
    FilterSet m_filters;
    RenderSet m_renders;
    pthread_t m_thread;
    IFilterEvent* m_pEvent;
    void* m_pTag;
    filter_graph_status m_status;
    bool m_isExit;
};

#endif // FILTERGRAPH_H
