#include "network.h"
#include "epoll.h"

CNetworkServer::CNetworkServer()
:m_callback(NULL)
,m_socket(INVALID_FD)
{
}

bool CNetworkServer::FinalConstruct(Interface* pOuter,void* pParam)
{
    m_callback = static_cast<IEpollCallback*>(pOuter);
    if(NULL != pParam)
        return IS_OK(Startup((url*)pParam)) ? true : false;
    else
        return true;
}

bool CNetworkServer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Shutdown();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CNetworkServer)
DOM_QUERY_IMPLEMENT(IStreamServer)
DOM_QUERY_IMPLEMENT(IEpollCallback)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CNetworkServer::Startup(url* pUrl)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;

    Shutdown();

    HRESULT hr = S_OK;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype =  SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    const char* pPort = NULL;
    if(NULL != pUrl)
        pPort = NULL == pUrl->port ? pUrl->protocol : pUrl->port;

    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr,0,len);

    if(NULL == pPort)
    {
        int type = SOCK_STREAM;
        if(m_callback != NULL)
            type |= SOCK_NONBLOCK;
        JCHK2(INVALID_FD != (m_socket = socket(AF_INET,type,0)),E_FAIL,
            "create socket fail,error id:%d msg:%s",errno,strerror(errno));
        addr.sin_family = AF_INET;
        addr.sin_port = 0;
        addr.sin_addr.s_addr = INADDR_ANY;
        JCHK3(0 == bind(m_socket, (sockaddr*)&addr, len),E_FAIL,
            "could not bind socket:%d error id:%d msg:%s",m_socket,errno,strerror(errno));
    }
    else
    {
        JCHK2(0 == (s = getaddrinfo(NULL, pPort, &hints, &result)),E_FAIL,
            "get local address info fail:%d msg:%s",s,gai_strerror(s));

        for(rp = result; rp != NULL; rp = rp->ai_next)
        {
            if(m_callback != NULL)
                rp->ai_socktype |= SOCK_NONBLOCK;
            m_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if(INVALID_FD == m_socket)
                continue;
            s = bind(m_socket, rp->ai_addr, rp->ai_addrlen);
            if(s == 0)
            {
                break;
            }
            LOG(0,"listen socket:%d bind port:%d fail",m_socket,ntohs(((sockaddr_in*)rp->ai_addr)->sin_port));
            sockaddr_in* addr = (sockaddr_in*)rp->ai_addr;
            addr->sin_port = 0;
            s = bind(m_socket, rp->ai_addr, rp->ai_addrlen);
            if(s == 0)
                break;
            close(m_socket);
            m_socket = INVALID_FD;
        }
        freeaddrinfo(result);
        JCHK2(NULL != rp,E_FAIL,"could not bind error id:%d msg:%s",errno,strerror(errno));
    }

    JCHK2(0 == listen(m_socket, SOMAXCONN),E_FAIL,"could not listen soket error id:%d msg:%s",errno,strerror(errno));

    if(-1 == getsockname(m_socket, (sockaddr*)&addr, &len))
    {
        LOG(0,"can not get soket address error id:%d msg:%s",errno,strerror(errno));
    }
    m_port = ntohs(addr.sin_port);
    if(NULL != m_callback)
    {
        JIF(m_callback->GetEpoll()->Add(m_socket,dynamic_cast<IEpollCallback*>(this)));
    }
    LOG(0,"server[%d] port:%d startup",m_socket,m_port);
    return hr ;
}

STDMETHODIMP_(void) CNetworkServer::Shutdown()
{
    if(INVALID_FD != m_socket)
    {
        if(NULL != m_callback)
            m_callback->GetEpoll()->Del(m_socket,dynamic_cast<IEpollCallback*>(this));
        LOG(0,"server[%d] port:%d shutdown",m_socket,m_port);
        close(m_socket);
        m_socket = INVALID_FD;
    }
}

STDMETHODIMP_(uint16_t) CNetworkServer::GetPort()
{
    return m_port;
}

STDMETHODIMP CNetworkServer::Accept(IStream** ppStream,IEpollCallback* pCallback)
{
    JCHK(NULL != ppStream,E_INVALIDARG);

    network_stream_param param;
    socklen_t in_len = sizeof(param.addr);
    param.fd = ::accept(m_socket,(sockaddr*)&param.addr,&in_len);
    if(INVALID_FD == param.fd)
    {
        JCHK2(EAGAIN == errno || EWOULDBLOCK == errno,E_FAIL,
            "could not accept error id:%d msg:%s",errno,strerror(errno));
        return E_AGAIN;
    }
    else
    {
        LOG(0,"server[%d]port:%d connect accept[%d]",m_socket,m_port,param.fd);
        dom_ptr<IStream> spStream;
        JCHK(spStream.Create(CLSID_CNetworkStream,pCallback,&param),E_FAIL);
        return spStream.CopyTo(ppStream);
    }
}

STDMETHODIMP CNetworkServer::OnEvent(uint32_t id,void* pParam)
{
    JCHK(NULL != m_callback,E_FAIL);
    HRESULT hr = S_OK;
    if(EPOLLIN == id)
    {
        //LOG(0,"server event in");
        hr = m_callback->OnEvent(IStreamServer::accept);
        //LOG(0,"server event out");
        return hr;
    }
    else if(0 != (EPOLLERR&id))
    {
        socklen_t len = sizeof(errno);
        int status = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &errno, &len);
        LOG(0,"socket status:%d error id:%d message:%s",status,errno,strerror(errno));
        hr = E_FAIL;
        m_callback->OnEvent(et_error,&hr);
    }
    return hr;
}

STDMETHODIMP_(IEpoll*) CNetworkServer::GetEpoll()
{
    return NULL != m_callback ? m_callback->GetEpoll() : NULL;
}

CNetworkStream::CNetworkStream()
:m_callback(NULL)
{
    //ctor
    memset(&m_addr,0,sizeof(m_addr));
}

bool CNetworkStream::FinalConstruct(Interface* pOuter,void* pParam)
{
    m_callback = static_cast<IEpollCallback*>(pOuter);
    if(NULL != pParam)
    {
        network_stream_param* param = static_cast<network_stream_param*>(pParam);
        m_addr = param->addr;
        JCHK(INVALID_FD != param->fd,E_INVALIDARG);
        m_stream.m_fd = param->fd;
        if(NULL != m_callback)
        {
            JCHK2(INVALID_FD != fcntl(m_stream.m_fd,F_SETFL,O_NONBLOCK),false,
            "could not set socket noblock error id:%d msg:%s",errno,strerror(errno));
            return  S_OK == m_callback->GetEpoll()->Add(m_stream.m_fd,dynamic_cast<IEpollCallback*>(this)) ? true : false;
        }
    }
    return true;
}

bool CNetworkStream::FinalDestructor(bool finally)
{
    if(true == finally)
        Close();
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CNetworkStream)
DOM_QUERY_IMPLEMENT(IStream)
DOM_QUERY_IMPLEMENT(IEpollCallback)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(uint32_t) CNetworkStream::GetFlag()
{
    return m_callback == NULL ? 0 : ANSY_FLAG;
}

STDMETHODIMP CNetworkStream::Open(url* pUrl,int mode)
{
    JCHK(NULL != pUrl,E_INVALIDARG);
    JCHK(NULL != pUrl->host,E_INVALIDARG);

    int s;
    addrinfo hints;
    memset(&hints,0,sizeof(hints));
    addrinfo *result, *rp;
    hints.ai_family = AF_INET;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;     /* All interfaces */
    const char* pPort = NULL == pUrl->port ? pUrl->protocol : pUrl->port;
    JCHK(NULL != pPort,E_INVALIDARG);

    JCHK4(0 == (s = getaddrinfo(pUrl->host, pPort, &hints, &result)),
        E_FAIL,"host:[%s] port:[%s] get local address info fail:%d msg:%s",pUrl->host,pPort,s,gai_strerror(s));

    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
        if(m_callback != NULL)
            rp->ai_socktype |= SOCK_NONBLOCK;
        sockaddr_in* addr = (sockaddr_in*)rp->ai_addr;
        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(INVALID_FD == s)
            continue;
        if(m_callback != NULL && IS_FAIL(m_callback->GetEpoll()->Add(s,dynamic_cast<IEpollCallback*>(this))))
        {
            close(s);
            continue;
        }
        if(0 == connect(s, rp->ai_addr, rp->ai_addrlen) || EINPROGRESS == errno)
        {
            m_stream.m_fd = s;
            m_stream.m_name = pUrl->host;
            m_addr = *addr;
            break;
        }
        m_callback->GetEpoll()->Del(s,dynamic_cast<IEpollCallback*>(this));
        close(s);
        LOG(1,"host:%s[%d] close",pUrl->host,s);
    }
    freeaddrinfo(result);
    JCHK4(NULL != rp,E_FAIL,"could not connect %s:%s error id:%d msg:%s",
        pUrl->host,pUrl->port,errno,strerror(errno));
    LOG(0,"host:%s[%d] connect",m_stream.m_name.c_str(),m_stream.m_fd);
    return S_OK;
}

STDMETHODIMP_(void) CNetworkStream::Close()
{
    if(INVALID_FD != m_stream.m_fd)
    {
        if(NULL != m_callback)
            m_callback->GetEpoll()->Del(m_stream.m_fd,dynamic_cast<IEpollCallback*>(this));
    }
    m_stream.Close();
}

STDMETHODIMP CNetworkStream::Read(void* pBuf,int32_t szBuf)
{
    HRESULT hr = m_stream.Read((uint8_t*)pBuf,szBuf);
    if(IS_FAIL(hr) && E_AGAIN != hr)
    {
        Close();
        if(NULL != m_callback)
            m_callback->OnEvent(et_error,&hr);
    }
    return hr;
}

STDMETHODIMP CNetworkStream::Write(void* pBuf,int32_t szBuf)
{
    HRESULT hr = m_stream.Write((uint8_t*)pBuf,szBuf);
    if(IS_FAIL(hr) && E_AGAIN != hr)
    {
        Close();
        if(NULL != m_callback)
            m_callback->OnEvent(et_error,&hr);
    }
    return hr;
}

STDMETHODIMP_(int64_t) CNetworkStream::Seek(int64_t position,seek_type type)
{
    JCHK0(false,E_FAIL,"network stream not support seek");
}

STDMETHODIMP_(int64_t) CNetworkStream::GetLength()
{
    return 0;
}

STDMETHODIMP CNetworkStream::SetLength(int64_t len)
{
    return 0;
}

STDMETHODIMP_(bool) CNetworkStream::IsOpen()
{
    return INVALID_FD != m_stream.m_fd;
}

STDMETHODIMP CNetworkStream::OnEvent(uint32_t id,void* pParam)
{
    JCHK(NULL != m_callback,E_FAIL);

    HRESULT hr = S_OK;

    if(0 != (EPOLLERR&id))
    {
        socklen_t len = sizeof(errno);
        int status = getsockopt(m_stream.m_fd, SOL_SOCKET, SO_ERROR, &errno, &len);
        LOG(0,"socket status:%d error id:%d message:%s",status,errno,strerror(errno));
        Close();
        hr = E_FAIL;
        m_callback->OnEvent(et_error,&hr);
    }
    else if(0 != (EPOLLHUP&id))
    {
        hr = E_FAIL;
        m_callback->OnEvent(et_error,&hr);
    }
    else if(0 != (EPOLLIN&id))
    {
        //LOG(0,"client read event in");
        hr = m_callback->OnEvent(IStream::read,NULL);
        //LOG(0,"client read event out");
    }
    else if(EPOLLOUT == id)
    {
        //LOG(0,"client write event in");
        hr = m_callback->OnEvent(IStream::write,NULL);
        //LOG(0,"client write event out");
    }
    return hr;
}

STDMETHODIMP_(IEpoll*) CNetworkStream::GetEpoll()
{
    return NULL != m_callback ? m_callback->GetEpoll() : NULL;
}
