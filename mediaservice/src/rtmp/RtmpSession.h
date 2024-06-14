#ifndef GOSUNSESSION_H
#define GOSUNSESSION_H
#include "stdafx.h"

#include "RtmpServer.hpp"
#include "RtmpClient.hpp"
#include "RtmpTimeJitter.hpp"

class CUrl;

class CRtmpSession : public ILoad, public IFilter , public ICallback
{
    typedef list< dom_ptr<IMediaFrame> > FrameSet;
    typedef FrameSet::iterator FrameIt;
public:
    DOM_DECLARE(CRtmpSession)
    //ILoad
    STDMETHODIMP Load(const char* pUrl,uint32_t mode,uint8_t flag);
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
    //CRtmpSession
    HRESULT SetType(FilterType type);

    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
    static HRESULT CreateListener(IStreamListen** ppListen,Interface* pObj,uint16_t* pPort);
protected:
    FilterType m_type;
    uint32_t m_flag;
    IFilter::Status m_status;

    void* m_pTag;
    string m_name;
    dom_ptr<IStream> m_spStream;
    dom_ptr<IInputPin> m_pinIn;
    dom_ptr<IOutputPin> m_pinOut;
    dom_ptr<IMediaFrameAllocate> m_spAllocate;
    dom_ptr<IMediaFrame> m_frameRecv;
    dom_ptr<IProfile> m_spProfile;
    FrameSet m_framesSend;
    dom_ptr<IEventPoint> m_ep;
    IStream::status m_stream_status;
protected:
    friend class RtmpServer;
    friend class RtmpClient;

public:
    HRESULT Recv();
    //HRESULT Recv(msg_type& type,IMediaFrame** pFrame);
    //HRESULT Send(msg_type type,IMediaFrame* pFrame = NULL);
    HRESULT Send();

protected:
    RtmpServer *m_server;
    RtmpClient *m_client;
    bool m_is_client;

    RtmpTimeJitter *m_jitter;
    bool m_correct;

    HRESULT send_message(IMediaFrame* pFrame, int64_t dts);
    HRESULT send_av_message(CommonMessage* msg);

    rtmp_request *generate_request(CUrl *curl);

    int32_t ProcessFlvSequenceHeader(const IMediaFrame::buf* pBuf, int64_t dts);

    dom_ptr<IStream> m_spRecord;

};

#endif // GOSUNSESSION_H
