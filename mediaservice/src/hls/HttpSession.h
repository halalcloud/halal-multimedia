#ifndef HTTPSESSION_H
#define HTTPSESSION_H
#include "stdafx.h"
#include <sstream>
#include <list>
#include <string>
#include <thread>
#include <mutex>
#include "HttpParser.hpp"

class CHttpSession : public ILoad, public IFilter, public ICallback
{
    typedef list< dom_ptr<IMediaFrame> > FrameSet;
    typedef FrameSet::iterator FrameIt;
public:
    DOM_DECLARE(CHttpSession)
    //ISession
    STDMETHODIMP Load();
    STDMETHODIMP_(void) Clear();
    //ILoad
    STDMETHODIMP Load(const char* pUrl,uint32_t mode,uint8_t flag = 0);
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
    HRESULT Recv(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    //HRESULT Recv(msg_type& type,IMediaFrame** pFrame);
//    HRESULT Send(msg_type type,IMediaFrame* pFrame = NULL);
    //HRESULT Send(IMediaFrame* pFrame = NULL);
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
    dom_ptr<ICallback> m_demuxer;
    FrameSet m_framesSend;
    CLocker m_locker;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;

    CUrl m_url;
    bool m_keep_alive;
    uint32_t m_mode;
    bool m_next;
    string m_file;
private:
    enum status
    {
        RECV_HTTP_H,
        RECV_CONTENT,
        RECV_CHUK_B,
        RECV_CHUK_H,
        RECV_DIRECT2,
    };
    string _buf;
    int64_t _contentLen;
    int64_t _alreadyRead;
    status _status;
    dom_ptr<IMediaFrame> _spFrame;
    HRESULT send_404();

    string get_response_header(int sc, const char* tp, uint32_t len = 0);
    void extraKeyValueWithRN(list<string> &kvPair);
    int64_t _offsetP;
    int64_t _offsetD;
    std::unique_ptr<HttpParser> _httpParser;
    int32_t sendCrossdomainXML();
    bool _isRes;
};

#endif // HTTPSESSION_H
