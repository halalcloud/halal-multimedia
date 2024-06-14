#ifndef TSDEMUXER_H
#define TSDEMUXER_H
#include "stdafx.h"
#include <map>
#include "./include/ITSDemuxer.h"

class M3u8Parser;
class CTsDemuxer : public IFilter, public ICallback
{
public:
    DOM_DECLARE(CTsDemuxer)
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
    //CTsDemuxer
    HRESULT Send(uint32_t cmd,bool down);
    HRESULT Open();
    HRESULT Close();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    void* m_tag;
    dom_ptr<IStream> m_spStream;
    dom_ptr<IInputPin> m_pinIn;
    dom_ptr<IOutputPin>* m_pinsOut;
    uint32_t m_countPin;
    IFilter::Status m_status;
    bool m_isEOF;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IEpollPoint> m_epoll;
    int64_t m_start;
    uint64_t m_clock;
private:
    HRESULT initDemuxer(IStream *strm);
    HRESULT getPacket(IStream *strm);
    MediaSubType stream_type2MediaSubType(int t);
    wzd::ITSDemuxer* _demuxer;
    std::map<int, dom_ptr<IOutputPin> > _id_pins;
    unsigned char _buf[188];
    unsigned short _bufLen;
    int _cnt;
    bool _init;
    enum STAT
    {
        RECV_M3u8,
        RECV_TS,
    };
    STAT _status;
    M3u8Parser* _m3u8Parser;
    IFilter* _session;
    bool _isM3u8;
    bool _isNewTSSeg;
    bool _hasV;
    string _m3u8_url;
    uint32_t m_duration;
    string _curTsSeg;
    int64_t _firstDTSofSEG;
    int64_t _lastDTSofSEG;
};

#endif // TSDEMUXER_H
