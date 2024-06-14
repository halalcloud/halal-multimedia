#ifndef FFMPEGVIDEOSCALE_H
#define FFMPEGVIDEOSCALE_H
#include "stdafx.h"


class CFFmpegVideoScale : public IFilter
{
public:
    DOM_DECLARE(CFFmpegVideoScale)
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
    //CFFmpegAudioEncoder
    HRESULT Write(IMediaFrame* pFrame);
    HRESULT Scale(IMediaFrame* pFrame,bool isSample);
    HRESULT Sample(IMediaFrame* pFrame,bool isScale);
    HRESULT SampleOut(IMediaFrame* pFrame,bool isScale);
    static HRESULT CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    IFilterGraphEvent* m_pFG;
    dom_ptr<IInputPin> m_spPinIn;
    dom_ptr<IOutputPin> m_spPinOut;
    bool m_isOpen;
    void* m_pTag;
	SwsContext* m_ctxSws;
    IMediaType* m_pMtIn;
    IMediaType* m_pMtOut;
    int m_height_in,m_height_out;
    int64_t m_duration_in,m_duration_out;
    dom_ptr<IMediaFrame> m_spFrameSample;
};

#endif // FFMPEGVIDEOSCALE_H
