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
        STDMETHODIMP GetMediaType(IMediaType* pMT);
        STDMETHODIMP_(IMediaType*) GetMediaType();
        STDMETHODIMP SetIndex(uint32_t index);
        STDMETHODIMP_(uint32_t) GetIndex();
        STDMETHODIMP Write(IMediaFrame* pFrame);
        STDMETHODIMP SetTag(void* pTag);
        STDMETHODIMP_(void*) GetTag();
        //IInputPin
        STDMETHODIMP Connect(IOutputPin* pPin,IMediaType* pMT);
        STDMETHODIMP_(void) Disconnect();
        STDMETHODIMP_(IOutputPin*) GetConnection();
        STDMETHODIMP_(IInputPin*) Next();
    protected:
        IFilter* m_pFilter;
        uint32_t m_index;
        void* m_pTag;
        ConnectionIt m_it;
        COutputPin* m_pPin;
        dom_ptr<IMediaType> m_spMT;
};

#endif // INPUTPIN_H
