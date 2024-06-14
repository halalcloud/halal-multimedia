#ifndef FFMPEGVIDEOSCALE_H
#define FFMPEGVIDEOSCALE_H
#include "stdafx.h"


class CFFmpegVideoScale : public IFilter
{
public:
    DOM_DECLARE(CFFmpegVideoScale)
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
    //CFFmpegAudioEncoder
    HRESULT Open();
    HRESULT Close();
    HRESULT Write(IMediaFrame* pFrame);
    HRESULT Scale(IMediaFrame* pFrame,bool isSample);
    HRESULT Sample(IMediaFrame* pFrame,bool isScale);
    HRESULT SampleOut(IMediaFrame* pFrame,bool isScale);
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    dom_ptr<IInputPin> m_spPinIn;
    dom_ptr<IOutputPin> m_spPinOut;
    IFilter::Status m_status;
    bool m_isOpen;
    void* m_pTag;
	SwsContext* m_ctxSws;
    int m_height_in,m_height_out;
    int64_t m_duration_in,m_duration_out;
    dom_ptr<IMediaFrame> m_spFrameSample;
    dom_ptr<IEventPoint> m_ep;
};

#endif // FFMPEGVIDEOSCALE_H
