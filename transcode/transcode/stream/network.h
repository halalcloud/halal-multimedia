#ifndef NETWORK_H
#define NETWORK_H

#include "stream.h"

struct network_stream_param
{
    int fd;
    sockaddr_in addr;
};

class CNetworkServer : public IStreamServer , public IEpollCallback
{
public:
    DOM_DECLARE(CNetworkServer)
    STDMETHODIMP Startup(url* pUrl);
    STDMETHODIMP_(void) Shutdown();
    STDMETHODIMP_(uint16_t) GetPort();
    STDMETHODIMP Accept(IStream** ppStream,IEpollCallback* pCallback);
    //IEpollCallback
    STDMETHODIMP OnEvent(uint32_t id,void* pParam);
    STDMETHODIMP_(IEpoll*) GetEpoll();
    //CNetworkServer
protected:
    IEpollCallback* m_callback;
    int m_socket;
    unsigned m_port;
};

class CNetworkStream : public IStream , public IEpollCallback
{
    friend class CNetworkServer;
public:
    DOM_DECLARE(CNetworkStream)
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP Open(url* pUrl,int mode);
    STDMETHODIMP_(void) Close();
    STDMETHODIMP Read(void* pBuf,int32_t szBuf);
    STDMETHODIMP Write(void* pBuf,int32_t szBuf);
    STDMETHODIMP_(int64_t) Seek(int64_t position,seek_type type);
    STDMETHODIMP_(int64_t) GetLength();
    STDMETHODIMP SetLength(int64_t len);
    STDMETHODIMP_(bool) IsOpen();
    //IEpollCallback
    STDMETHODIMP OnEvent(uint32_t id,void* pParam);
    STDMETHODIMP_(IEpoll*) GetEpoll();
    //CTcpStream
protected:
    HRESULT Recv(void* pBuf,size_t len);
    HRESULT Send(void* pBuf,size_t len);
protected:
    IEpollCallback* m_callback;
    CStream m_stream;
    sockaddr_in m_addr;
};

#endif // NETWORK_H
