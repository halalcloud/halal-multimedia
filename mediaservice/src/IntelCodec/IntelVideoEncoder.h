#ifndef INTELVIDEOENCODER_H
#define INTELVIDEOENCODER_H
#include "stdafx.h"

#include "mfx_samples_config.h"
#include <iostream>
#include <memory>
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include <stdarg.h>
#include <string>

class CIntelVideoEncoder : public IFilter
{
public:
    DOM_DECLARE(CIntelVideoEncoder)
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
    //CIntelDecoder
    HRESULT Open();
    HRESULT Close();
    HRESULT Write(IMediaFrame* pFrame);
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    dom_ptr<IInputPin> m_spPinIn;
    dom_ptr<IOutputPin> m_spPinOut;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
    IFilter::Status m_status;
    bool m_isOpen;
    void* m_pTag;
    VideoMediaType m_pix_fmt;
    int m_width;
    int m_height;
    int m_ratioX;
    int m_ratioY;
    int m_align;
    int64_t m_duration;
private:
    void GetOption(IProfile* pProfile);
    sInputParams Params;
    std::auto_ptr<CEncodingPipeline>  pPipeline;
                outter otr;
    mfxStatus sts = MFX_ERR_NONE; // return value check
};

#endif // INTELVIDEOENCODER_H
