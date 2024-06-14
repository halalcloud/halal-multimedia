#ifndef INPUTPIN_H
#define INPUTPIN_H
#include "stdafx.h"
#include "OutputPin.h"

class CInputPin : public IInputPin
{
    public:
        DOM_DECLARE(CInputPin)
        //IPin
        STDMETHODIMP_(IFilter*) GetFilter();
        STDMETHODIMP_(IMediaType*) GetMediaType();
        STDMETHODIMP SetMediaType(IMediaType* pMT);
        STDMETHODIMP SetIndex(uint32_t index);
        STDMETHODIMP_(uint32_t) GetIndex();
        STDMETHODIMP_(void)SetID(uint32_t id);
        STDMETHODIMP_(uint32_t)GetID();
        STDMETHODIMP Send(uint32_t cmd,bool down,bool first);
        STDMETHODIMP_(uint32_t) Recv();
        STDMETHODIMP_(void) Set(uint32_t cmd);
        STDMETHODIMP Write(IMediaFrame* pFrame);
        STDMETHODIMP_(void) SetTag(void* pTag);
        STDMETHODIMP_(void*) GetTag();
        STDMETHODIMP_(void) SetObj(Interface* pObj);
        STDMETHODIMP_(Interface*) GetObj();
        STDMETHODIMP SetFlag(uint8_t flag);
        //IInputPin
        STDMETHODIMP Connect(IOutputPin* pPin,IMediaType* pMT);
        STDMETHODIMP_(void) Disconnect();
        STDMETHODIMP_(IOutputPin*) GetConnection();
        STDMETHODIMP_(IInputPin*) Next();
        STDMETHODIMP Pop(IMediaFrame** ppFrame);
        STDMETHODIMP_(int64_t) GetBufLen();
        STDMETHODIMP_(bool) IsEnd();
   protected:
        IFilter* m_pFilter;
        uint32_t m_index;
        uint32_t m_id;
        void* m_pTag;
        dom_ptr<Interface> m_obj;
        ConnectionIt m_it;
        COutputPin* m_pPin;
        uint32_t m_cmd;
        bool m_is_end;
        dom_ptr<IMediaType> m_spMT;
        FrameSet m_frames;
        int64_t m_len;
        uint32_t m_key;
        uint8_t m_flag;
        CLocker m_locker;
};

#endif // INPUTPIN_H
