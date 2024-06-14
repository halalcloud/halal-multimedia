#ifndef FILTERGRAPH_H
#define FILTERGRAPH_H
#include <iostream>
#include <map>
#include <list>

#include "stdafx.h"

class CFilterGraph : public IFilterGraph ,public ICallback
{
    typedef list< dom_ptr<IFilter> > FilterSet;
    typedef FilterSet::iterator FilterIt;

public:
    DOM_DECLARE(CFilterGraph)
    //IFilterGraph
    STDMETHODIMP Create(FilterType type,const char* pUrl,IFilter** ppFilter,const char* pFormat,const char* pName);
    STDMETHODIMP ConnectPin(IOutputPin* pPinOut,IInputPin* pPinIn,IMediaType* pMT);
    STDMETHODIMP ConnectPin(IOutputPin* pPin,IFilter** ppFilter,const char* pName);
    STDMETHODIMP ConnectPin(IInputPin* pPin,IFilter** ppFilter,const char* pName);
    STDMETHODIMP Append(IFilter* pFilter);
    STDMETHODIMP Enum(FilterType type,IIt** ppIt);
    STDMETHODIMP_(IFilter*)Get(IIt* pIt);
    STDMETHODIMP_(uint32_t) GetCount(FilterType type);
    STDMETHODIMP Notify(IFilter* pFilter,uint32_t cmd,bool first);
    STDMETHODIMP_(Status&) GetStatus();
    STDMETHODIMP Clear();
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
protected:
    HRESULT ConnectPin(IOutputPin* pPinOut,IInputPin* pPinIn,IMediaType* pMT,bool isDrop);
    HRESULT Append(FilterType type,IFilter* pFilter);
protected:
    dom_ptr<SET(FilterSet)> m_sources;
    dom_ptr<SET(FilterSet)> m_renders;
    dom_ptr<SET(FilterSet)> m_filters;
    ICallback* m_callback;
    Status m_status;
};

#endif // FILTERGRAPH_H
