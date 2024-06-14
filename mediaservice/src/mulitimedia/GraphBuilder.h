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

class CGraphBuilder : public IGraphBuilder,public ICallback
{
public:
    DOM_DECLARE(CGraphBuilder)
    //IGraphBuilder
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP Set(IFilter* pFilter);
    STDMETHODIMP Load(const char* pTask);
    STDMETHODIMP_(void) Clear();
    STDMETHODIMP PublishPointAddClient(IOutputPin* pPinOut,IInputPin* pPinIn);
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
protected:
    HRESULT Play(IFilter* pFilter);
    HRESULT Connect(IFilter* pUpper,IFilter* pDown);
    HRESULT BuildOutput(Object::Ptr& channel);
    IOutputPin* Find(IFilter* pFilter,IMediaType* pMT,IOutputPin* pSame,uint32_t& level);
//    Stream* Find(uint32_t index,const Object::Ptr& stream);
    static HRESULT Convert(IFilter* pFilter,const Object::Ptr& obj);
    static HRESULT Convert(IMediaType* pMT,const Object::Ptr& obj);
//    static IOutputPin* Find(IFilter* pFilter,MediaMajorType major,IMediaType** ppMt,uint32_t index);
//    static IOutputPin* Find(IOutputPin* pPin,bool isCompress);
//    static HRESULT GetDuration(IProfile* pProfile,int64_t& duration);
//    void Process();
//    static void* Process(void* pParam);
    bool GetOutputUrl(string& format,const tm* tm = NULL,int64_t* timestamp = NULL,uint32_t* index = NULL);
    static bool CreateDirectory(const string& path);
//    HRESULT GetInputInfo(Object::Ptr obj,IFilter* pFilter,HRESULT hr);
//    void GetInfo(Object::Ptr obj,IProfile* pProfile);
protected:
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IEpoll> m_spEpoll;

    dom_ptr<IFilterGraph> m_spFG;
    dom_ptr<IFilter> m_spInput;
    dom_ptr<IFilter> m_spOutput;
    dom_ptr<IProfile> m_spProfile;
    dom_ptr<IInputPin> m_spInputPin;

    Array::Ptr m_outputs;
    Array::Ptr m_template;

    int64_t m_time;
    int64_t m_start;

    string m_stream;
    CLocker m_locker;
};

#endif // GRAPHBUILDER_H
