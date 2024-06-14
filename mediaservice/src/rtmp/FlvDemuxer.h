#ifndef FLVDEMUXER_H
#define FLVDEMUXER_H
#include "stdafx.h"
#include "RtmpGlobal.hpp"

class CFlvDemuxer : public IFilter , public ICallback
{
public:
    DOM_DECLARE(CFlvDemuxer)
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
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    //CFlvDemuxer
    HRESULT Send(uint32_t cmd,bool down);
    HRESULT Open();
    HRESULT Close();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    void* m_tag;
    dom_ptr<IProfile> m_spProfile;
    dom_ptr<IStream> m_spStream;
    dom_ptr<IInputPin> m_pinIn;

    dom_ptr<IOutputPin> m_pinsOut_audio;
    dom_ptr<IOutputPin> m_pinsOut_video;

    uint32_t m_countPin;
    IFilter::Status m_status;
    bool m_isEOF;
    dom_ptr<IEventPoint> m_ep;

    HRESULT CreatePinOuts(IMediaType* pMT);
    HRESULT CreatePinOuts();
    HRESULT ProcessFlvFrame(IMediaFrame* pFrame);

    HRESULT ReadFlvHeader(IStream *pStream);
    HRESULT ReadFlvTagHeader(IStream *pStream);
    HRESULT ReadFlvTagBody(IStream *pStream);
    HRESULT ReadFlvTagPrevious(IStream *pStream);
    HRESULT FlvService(IStream *pStream);

    enum Schedule {
        FlvHeader = 0,
        TagHeader,
        TagBody,
        PrevousTagSize
    };
    int8_t m_type;


    list<CommonMessage*> m_msgs;
    bool m_first;

    CommonMessageHeader m_header;

    dom_ptr<IMediaFrameAllocate> m_spAllocate;


private:
    int process_v_sequence(CommonMessage *msg);
    int process_a_sequence(CommonMessage *msg);
    int process_metadata(CommonMessage *msg);
    int process_av_cache();
    int process_av_data(CommonMessage *msg);
    int process_video_audio(CommonMessage *msg);
    void clear();
};

#endif // FLVDEMUXER_H
