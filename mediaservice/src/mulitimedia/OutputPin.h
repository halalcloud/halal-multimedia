#ifndef OUTPUTPIN_H
#define OUTPUTPIN_H
#include <vector>
#include "stdafx.h"

using namespace std;
typedef list< dom_ptr<IInputPin> > ConnectionSet;
typedef ConnectionSet::iterator ConnectionIt;
class COutputPin : public IOutputPin , public ICallback
{
    public:
        DOM_DECLARE(COutputPin)
        //IPin
        STDMETHODIMP_(IFilter*) GetFilter();
        STDMETHODIMP_(IMediaType*) GetMediaType();
        STDMETHODIMP SetMediaType(IMediaType* pMT);
        STDMETHODIMP SetIndex(uint32_t index);
        STDMETHODIMP_(uint32_t) GetIndex();
        STDMETHODIMP_(void)SetID(uint32_t id);
        STDMETHODIMP_(uint32_t)GetID();
        STDMETHODIMP Send(uint32_t cmd,bool down,bool first);
        STDMETHODIMP_(uint32_t)Recv();
        STDMETHODIMP_(void) Set(uint32_t cmd);
        STDMETHODIMP Write(IMediaFrame* pFrame);
        STDMETHODIMP_(void) SetTag(void* pTag);
        STDMETHODIMP_(void*) GetTag();
        STDMETHODIMP_(void) SetObj(Interface* pObj);
        STDMETHODIMP_(Interface*) GetObj();
        STDMETHODIMP SetFlag(uint8_t flag);
        //IOutputPin
        STDMETHODIMP Connect(IInputPin* pPin,IMediaType* pMT);
        STDMETHODIMP_(void) Disconnect(IInputPin* pPin);
        STDMETHODIMP_(IInputPin*) GetConnection();
        STDMETHODIMP AllocFrame(IMediaFrame** ppFrame);
        STDMETHODIMP SetClock(bool enable);
        //ICallback
        STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
        //COutputPin
        ConnectionIt Connect(IInputPin* pPin);
        void Disconnect(ConnectionIt it);
        IInputPin* Next(ConnectionIt it);
        HRESULT Render(IMediaFrame* pFrame);
    protected:
        IFilter* m_pFilter;
        uint32_t m_index;
        uint32_t m_id;
        void* m_pTag;
        dom_ptr<Interface> m_obj;
        dom_ptr<IMediaType> m_spMT;
        dom_ptr<IMediaFrameAllocate> m_allocate;
        dom_ptr<IEpollPoint> m_ep;
        FrameSet m_frames;
        CLocker m_locker;
        ConnectionSet m_connections;
        uint32_t m_ref;
        uint32_t m_cmd;
        int64_t m_start;
        int64_t m_clock;
        int64_t m_segment;
        int64_t m_start_segment;
        uint8_t m_flag;
        ConnectionIt m_it;
//#define PIN_OUTPUT
#ifdef PIN_OUTPUT
        FILE* m_dump;
#endif
};

#endif // OUTPUTPIN_H
