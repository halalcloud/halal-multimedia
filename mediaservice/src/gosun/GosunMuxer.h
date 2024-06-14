#ifndef GOSUNMUXER_H
#define GOSUNMUXER_H
#include <vector>
#include "stdafx.h"


class CGosunMuxer : public IFilter
{
    typedef vector< dom_ptr<IInputPin> > InputSet;
    typedef InputSet::iterator InputIt;
    typedef list< dom_ptr<IMediaFrame> > FrameSet;
    typedef FrameSet::iterator FrameIt;
public:
    DOM_DECLARE(CGosunMuxer)
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
    //CGosunMuxer
    HRESULT Deliver(uint32_t id,IMediaFrame* pFrame);
    HRESULT Open();
    HRESULT Close();
    HRESULT Output(IMediaFrame* pFrame);
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    InputSet m_inputs;
    dom_ptr<IOutputPin> m_pinOut;
    IFilter::Status m_status;
    bool m_isOpen;
    bool m_isFirst;
    void* m_pTag;
    uint32_t m_count;
    int64_t m_start_time;
    int64_t m_delta_time;
    dom_ptr<IProfile> m_spProfile;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IMediaFrame> m_frame;
    CLocker m_locker;
};

#endif // GOSUNMUXER_H
