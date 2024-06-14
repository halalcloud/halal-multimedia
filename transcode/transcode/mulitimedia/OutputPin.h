#ifndef OUTPUTPIN_H
#define OUTPUTPIN_H
#include <vector>
#include "stdafx.h"

using namespace std;
typedef list< dom_ptr<IInputPin> > ConnectionSet;
typedef ConnectionSet::iterator ConnectionIt;
class COutputPin : public IOutputPin
{
    public:
        DOM_DECLARE(COutputPin)
        //IPin
        STDMETHODIMP_(IFilter*) GetFilter();
        STDMETHODIMP GetMediaType(IMediaType* pMT);
        STDMETHODIMP_(IMediaType*) GetMediaType();
        STDMETHODIMP SetIndex(uint32_t index);
        STDMETHODIMP_(uint32_t) GetIndex();
        STDMETHODIMP Write(IMediaFrame* pFrame);
        STDMETHODIMP SetTag(void* pTag);
        STDMETHODIMP_(void*) GetTag();
        //IOutputPin
        STDMETHODIMP Connect(IInputPin* pPin,IMediaType* pMT);
        STDMETHODIMP Disconnect(IInputPin* pPin);
        STDMETHODIMP_(IInputPin*) GetConnection();
        STDMETHODIMP_(void) NewSegment();
        STDMETHODIMP AllocFrame(IMediaFrame** ppFrame);
        //COutputPin
        ConnectionIt Connect(IInputPin* pPin);
        void Disconnect(ConnectionIt& it);
        IInputPin* Next(ConnectionIt it);
    protected:
        IFilter* m_pFilter;
        uint32_t m_index;
        void* m_pTag;
        dom_ptr<IMediaType> m_spMT;
        dom_ptr<IMediaFrameAllocate> m_allocate;
        ConnectionSet m_connections;
        ConnectionIt m_it;
        int64_t m_pos;
        bool m_is_new_segment;
//#define PIN_OUTPUT
#ifdef PIN_OUTPUT
        FILE* m_dump;
#endif
};

#endif // OUTPUTPIN_H
