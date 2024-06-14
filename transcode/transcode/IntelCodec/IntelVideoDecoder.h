#ifndef INTELVIDEODECODER_H
#define INTELVIDEODECODER_H
#include "stdafx.h"

class CIntelVideoDecoder : public IFilter
{
public:
    DOM_DECLARE(CIntelVideoDecoder)
    //IFilter
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(uint32_t) GetInputPinCount();
    STDMETHODIMP_(IInputPin*) GetInputPin(uint32_t index);
    STDMETHODIMP_(uint32_t) GetOutputPinCount();
    STDMETHODIMP_(IOutputPin*) GetOutputPin(uint32_t index);
    STDMETHODIMP Open();
    STDMETHODIMP Close();
    STDMETHODIMP SetTag(void* pTag);
    STDMETHODIMP_(void*) GetTag();
    STDMETHODIMP_(double) GetExpend();
    STDMETHODIMP OnGetMediaType(IInputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnGetMediaType(IOutputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnSetMediaType(IInputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnSetMediaType(IOutputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    STDMETHODIMP OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame);
    //CIntelVideoDecoder
    HRESULT Write(IMediaFrame* pFrame);
    static HRESULT CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    IFilterGraphEvent* m_pFG;
    dom_ptr<IInputPin> m_spPinIn;
    dom_ptr<IOutputPin> m_spPinOut;
    dom_ptr<IMediaFrame> m_spFrame;
    bool m_isOpen;
    void* m_pTag;
    IMediaType* m_pMtIn;
    IMediaType* m_pMtOut;
    bool m_isFirst;
    VideoMediaType m_pix_fmt;
    int m_width;
    int m_height;
    int m_ratioX;
    int m_ratioY;
    int m_align;
    int64_t m_duration;
};

#endif // INTELVIDEODECODER_H

