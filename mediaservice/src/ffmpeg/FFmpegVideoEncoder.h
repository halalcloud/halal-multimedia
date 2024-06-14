#ifndef FFMPEGVIDEOENCODER_H
#define FFMPEGVIDEOENCODER_H
#include "stdafx.h"


class CFFmpegVideoEncoder : public IFilter
{
public:
    DOM_DECLARE(CFFmpegVideoEncoder)
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
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    dom_ptr<IInputPin> m_spPinIn;
    dom_ptr<IOutputPin> m_spPinOut;
    IFilter::Status m_status;
    bool m_isOpen;
    void* m_pTag;
    AVCodecContext* m_ctxCodec;
    int64_t m_duration;
    int m_ratioX;
    int m_ratioY;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
};

#endif // FFMPEGVIDEOENCODER_H
