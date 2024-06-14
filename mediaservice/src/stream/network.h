#ifndef NETWORK_H
#define NETWORK_H
#include "stdafx.h"

struct network_stream_param
{
    int fd;
    sockaddr_in addr;
};

struct write_buf
{
    uint8_t* buf;
    uint32_t len;
    write_buf():buf(NULL),len(0){}
    ~write_buf()
    {
        if(NULL != buf)
        {
            free(buf);
            buf = NULL;
        }
        len = 0;
    }
    HRESULT set(const void* pBuf,uint32_t size)
    {
        JCHK(buf = (uint8_t*)realloc(buf,size),E_OUTOFMEMORY);
        memcpy(buf,pBuf,size);
        len = size;
        return S_OK;
    }
};

class CNetworkListen : public IStreamListen , public ICallback
{
public:
    DOM_DECLARE(CNetworkListen)
    //IStreamListen
    STDMETHODIMP Startup(const ClassInfo* info,uint16_t port);
    STDMETHODIMP Accept(IFilter** ppFilter);
    STDMETHODIMP_(void) Shutdown();
    //ICallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
protected:
    int m_fd;
    uint16_t m_port;
    const ClassInfo* m_class;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IEpollPoint> m_epoll;
    ILocker* m_locker;
};

class CNetworkStream : public IStream , public ICallback
{
    friend class CNetworkListen;
public:
    DOM_DECLARE(CNetworkStream)
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP Open(const char*  pUrl,uint32_t mode);
    STDMETHODIMP_(void) Close();
    STDMETHODIMP SetEventEnable(bool enable);
    STDMETHODIMP_(bool) GetEventEnable();
    STDMETHODIMP SetTimer(int id,uint32_t duration,bool one_shoot);
    STDMETHODIMP_(bool) CanRead();
    STDMETHODIMP_(bool) CanWrite();
    STDMETHODIMP Read(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf);
    STDMETHODIMP Write(const void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf);
    STDMETHODIMP Flush();
    STDMETHODIMP_(int64_t) Seek(int64_t position,seek_type type);
    STDMETHODIMP_(int64_t) GetPos();
    STDMETHODIMP_(int64_t) GetLength();
    STDMETHODIMP SetLength(int64_t len);
    STDMETHODIMP_(bool) IsOpen();
    STDMETHODIMP_(status&) GetStatus();
    //IEpollCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    //CNetworkStream
    HRESULT Recv(uint8_t* pBuf,uint32_t szBuf,uint32_t& pos,uint8_t flag);
    HRESULT Send(const uint8_t*pBuf,uint32_t szBuf,uint32_t& pos,uint8_t flag);
    HRESULT Send(IMediaFrame* pFrame,uint32_t& index,uint32_t& offset,uint8_t flag);
protected:
    ICallback* m_callback;
    int m_fd;
    sockaddr_in m_addr;
    string m_name;
    uint8_t* m_bufRead;
    uint32_t m_lenRead;
    uint32_t m_begRead;
    uint32_t m_endRead;
    uint8_t m_flagRead;

    uint8_t* m_bufWrite;
    uint32_t m_lenWrite;
    uint32_t m_posWrite;
    uint32_t m_indexWrite;
    uint32_t m_offsetWrite;
    uint8_t m_flagWrite;

    status m_status;
    dom_ptr<IMediaFrame> m_frame;
    IMediaFrame::buf* m_buf;
    uint32_t m_flag;
    uint32_t m_timer;
    dom_ptr<IEpollPoint> m_ep;
    ILocker* m_locker;
    bool m_event;
    int m_thread_read;
};

#endif // NETWORK_H
