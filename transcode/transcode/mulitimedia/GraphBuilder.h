#ifndef GRAPHBUILDER_H
#define GRAPHBUILDER_H

#include "stdafx.h"
#include "iGraphBuilder.h"

struct Status
{
    bool isRun;
    bool isLive;
    bool isCycle;
    uint32_t number;
    uint32_t count;
    double progress;
    double speed;
    int64_t clock;
    int64_t time;
    int64_t buffer;
};

class CGraphBuilder : public IGraphBuilder,public IFilterEvent
{
    struct Stream
    {
        Stream(const Object::Ptr& stream)
        :stream(stream){}
        Object::Ptr stream;
        dom_ptr<IOutputPin> spPin;
    };
    typedef multimap< uint32_t,Stream > StreamSet;
    typedef StreamSet::iterator StreamIt;
    typedef pair<StreamSet::key_type,StreamSet::mapped_type> StreamPair;

    typedef list< dom_ptr<IFilter> > FilterSet;
    typedef FilterSet::iterator FilterIt;
public:
    DOM_DECLARE(CGraphBuilder)
    //IGraphBuilder
    STDMETHODIMP Play(const char* pTask,bool isWait = false,IGraphBuilderCallback* pCallback = NULL);
    STDMETHODIMP Stop();
    STDMETHODIMP_(bool) IsRunning();
    STDMETHODIMP_(const char*) GetInfo(info_type it);
    //IFilterEvent
    STDMETHODIMP OnEvent(EventType type,HRESULT hr,IFilter* pFilter,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame,void* pTag);
protected:
    const Status& GetStatus();
    HRESULT Next(HRESULT hr);
    HRESULT SetInputBeg(Array::ValueVec::const_iterator& it);
    HRESULT SetInputEnd();
    HRESULT SetInput(IFilter* pFilter);
    HRESULT AddOutput(const Array::Ptr& streams,Object::Ptr& profiles);
    Stream* Find(uint32_t index,const Object::Ptr& stream);
    static HRESULT Convert(IFilter* pFilter,const Object::Ptr& obj);
    static HRESULT Convert(IMediaType* pMT,const Object::Ptr& obj);
    static HRESULT Convert(IProfile* pProfile,const Object::Ptr& obj);
    static IOutputPin* Find(IFilter* pFilter,MediaMajorType major,IMediaType** ppMt,uint32_t index);
    static IOutputPin* Find(IOutputPin* pPin,bool isCompress);
    static HRESULT GetDuration(IProfile* pProfile,int64_t& duration);
    void Remove(IFilter* pInput);
    void Process();
    static void* Process(void* pParam);
    bool GetOutputUrl(string& format,int64_t timestamp,uint32_t index);
    bool CreateDirectory(const string& path);
    HRESULT GetInputInfo(Object::Ptr obj,IFilter* pFilter,HRESULT hr);
    void GetInfo(Object::Ptr obj,IProfile* pProfile);
    static HRESULT dump_callback(HRESULT hr,const char* pModule,const char* pContent,void* pTag);
protected:
    dom_ptr<IFilterGraph> m_spFG;
    dom_ptr<IFilter> m_spInput;
    dom_ptr<IFilter> m_spNewInput;

    Array::Ptr m_inputs;
    Array::Ptr m_outputs;
    StreamSet m_streams;
    FilterSet m_renders;

    Array::ValueVec::const_iterator m_itInput;
    Array::ValueVec::const_iterator m_itNewInput;
    int64_t m_start;
    IGraphBuilderCallback* m_callback;
    int64_t m_duration;
    int64_t m_begin;
    Status m_status;
    string m_info[it_nb];
    string m_msg_error;
};

#endif // GRAPHBUILDER_H
