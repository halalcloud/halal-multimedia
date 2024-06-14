#ifndef FFMPEGAUDIORESAMPLE_H
#define FFMPEGAUDIORESAMPLE_H
#include "stdafx.h"


class CFFmpegAudioResample : public IFilter
{
public:
    DOM_DECLARE(CFFmpegAudioResample)
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
    HRESULT Sample(IMediaFrame* pFrame);
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    dom_ptr<IInputPin> m_spPinIn;
    dom_ptr<IOutputPin> m_spPinOut;
    IFilter::Status m_status;
    bool m_isOpen;
    void* m_pTag;
    dom_ptr<IMediaType> m_spMt;
    IMediaType* m_pMtResample;
    SwrContext* m_ctxResample;
    int m_frame_size_resample;
    int m_frame_size_out;
    AudioMediaType m_sample_fmt_out;
    int m_channel_out;
    MEDIA_FRAME_INFO m_info;
    dom_ptr<IMediaFrame> m_spFrameSample;
    dom_ptr<IEventPoint> m_ep;
};

#endif // FFMPEGAUDIORESAMPLE_H
