#ifndef FFMPEGAUDIORESAMPLE_H
#define FFMPEGAUDIORESAMPLE_H
#include "stdafx.h"


class CFFmpegAudioResample : public IFilter
{
public:
    DOM_DECLARE(CFFmpegAudioResample)
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
    HRESULT Sample(IMediaFrame* pFrame);
    static HRESULT CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    IFilterGraphEvent* m_pFG;
    dom_ptr<IInputPin> m_spPinIn;
    dom_ptr<IOutputPin> m_spPinOut;
    bool m_isOpen;
    void* m_pTag;
    IMediaType* m_pMtIn;
    IMediaType* m_pMtOut;
    dom_ptr<IMediaType> m_spMt;
    IMediaType* m_pMtResample;
    SwrContext* m_ctxResample;
    int m_frame_size_resample;
    int m_frame_size_out;
    AudioMediaType m_sample_fmt_out;
    int m_channel_out;
    MEDIA_FRAME_INFO m_info;
    dom_ptr<IMediaFrame> m_spFrameSample;
};

#endif // FFMPEGAUDIORESAMPLE_H
