#ifndef GOSUNMUXER_H
#define GOSUNMUXER_H
#include <vector>
#include "stdafx.h"


class CFlvMuxer : public IFilter
{
    typedef vector< dom_ptr<IInputPin> > InputSet;
    typedef InputSet::iterator InputIt;
public:
    DOM_DECLARE(CFlvMuxer)
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
    //CFlvMuxer
    HRESULT Open();
    HRESULT Close();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    InputSet m_inputs;
    dom_ptr<IOutputPin> m_pinOut;
    vector<uint32_t> m_indexs;
    dom_ptr<IMediaFrameAllocate> m_spAllocate;
    IFilter::Status m_status;
    bool m_isOpen;
    bool m_isFirst;
    void* m_pTag;
    uint32_t m_index;
    uint32_t m_master;
    dom_ptr<IProfile> m_spProfile;
    dom_ptr<IEventPoint> m_ep;

    bool m_v_sequence;
    bool m_a_sequence;

    bool m_v_convert;
    bool m_a_convert;

    int64_t m_base;
    // generate metadata and audio/video sequence header to flv tag
    void generate_flv_sm_tag(const IMediaFrame::buf *pBuf, uint8_t *extra_data, int extra_size, char type);

    int generate_flv_tag(IMediaFrame *pFrame, bool is_video, bool is_264, bool is_aac,
                          uint8_t *buf, int size, int64_t dts, int64_t pts, bool is_keyframe);
};

#endif // GOSUNMUXER_H
