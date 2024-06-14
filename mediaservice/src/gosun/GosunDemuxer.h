#ifndef GOSUNDEMUXER_H
#define GOSUNDEMUXER_H
#include "stdafx.h"

class CGosunDemuxer : public IFilter
{
public:
    DOM_DECLARE(CGosunDemuxer)
    //IFilter
    STDMETHODIMP_(FilterType) GetType();
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(uint32_t) GetInputPinCount();
    STDMETHODIMP_(IInputPin*) GetInputPin(uint32_t index);
    STDMETHODIMP_(uint32_t) GetOutputPinCount();
    STDMETHODIMP_(IOutputPin*) GetOutputPin(uint32_t index);
    STDMETHODIMP_(IInputPin*) CreateInputPin(IMediaType* pMT);
    STDMETHODIMP_(IOutputPin*) CreateOutputPin(IMediaType* pMT);
    STDMETHODIMP Notify(uint32_t cmd);
    STDMETHODIMP_(uint32_t) GetStatus();
    STDMETHODIMP_(void) SetTag(void* pTag);
    STDMETHODIMP_(void*) GetTag();
    STDMETHODIMP_(double) GetExpend();
    STDMETHODIMP OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT);
    STDMETHODIMP OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    //ISerialize
    HRESULT Load(IStream* pStream,uint8_t flag);
    //CGosunDemuxer
    HRESULT Send(uint32_t cmd,bool down);
    HRESULT Open();
    HRESULT Close();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    void* m_tag;
    dom_ptr<IStream> m_spStream;
    dom_ptr<IInputPin> m_pinIn;
    dom_ptr<IOutputPin>* m_pinsOut;
    uint32_t m_count;
    IFilter::Status m_status;
    bool m_isOpen;
    bool m_isEOF;
    dom_ptr<IEventPoint> m_ep;
};

#endif // GOSUNDEMUXER_H
