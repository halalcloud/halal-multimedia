#include "network.h"
#include "epoll.h"
#include <arpa/inet.h>

CNetworkListen::CNetworkListen()
:m_fd(INVALID_FD)
,m_port(0)
,m_class(NULL)
,m_locker(NULL)
{
}


bool CNetworkListen::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IStreamListen*)this,true,pParam),false);

    dom_ptr<IEpoll> spEpoll;
    JCHK(spEpoll.QueryFrom(g_pSite->GetObj()),false);
    JCHK(S_OK == spEpoll->CreatePoint(this,&m_epoll),false);
    JCHK(NULL != (m_locker = m_epoll->GetLocker()),false);
    return true;
}

bool CNetworkListen::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Shutdown();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CNetworkListen)
DOM_QUERY_IMPLEMENT(IStreamListen)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CNetworkListen::Startup(const ClassInfo* info,uint16_t port)
{
    JCHK(NULL != info,E_INVALIDARG);
    JCHK(STR_CMP(FILTER_TYPE_NAME,info->major),E_INVALIDARG);
    JCHK(FT_Session == info->sub,E_INVALIDARG);

    CLocker locker(m_locker);
    HRESULT hr;

    Shutdown();

    struct addrinfo hints;


    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype =  SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr,0,len);

    int type = SOCK_STREAM|SOCK_NONBLOCK;
    int fd;
    JCHK2(INVALID_FD != (fd = socket(AF_INET,type,0)),E_FAIL,
        "create socket fail,error id:%d msg:%s",errno,strerror(errno));

    addr.sin_family = AF_INET;
    addr.sin_port = 0 == port ? 0 : htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int reuse_socket = 1;
    JCHK3(-1 != setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_socket, sizeof(reuse_socket)),E_FAIL,
        "set fd:%d reuse-addr fail,error id:%d msg:%s",fd,errno,strerror(errno));

    JCHK3(0 == bind(fd, (sockaddr*)&addr, len),E_FAIL,
            "could not bind socket:%d error id:%d msg:%s",fd,errno,strerror(errno));

    JCHK2(0 == listen(fd, SOMAXCONN),E_FAIL,"could not listen soket error id:%d msg:%s",errno,strerror(errno));

    if(-1 == getsockname(fd, (sockaddr*)&addr, &len))
    {
        LOG(0,"can not get soket address error id:%d msg:%s",errno,strerror(errno));
    }
    m_class = info;
    m_port = ntohs(addr.sin_port);

    JIF(m_epoll->Add(fd));
    m_fd = fd;
    //LOG(0,"server[%d] port:%d startup",m_fd,m_port);
    return hr;
}

STDMETHODIMP_(void) CNetworkListen::Shutdown()
{
    CLocker locker(m_locker);

    if(INVALID_FD != m_fd)
    {
        m_epoll->Del(m_fd);
        close(m_fd);
        m_fd = INVALID_FD;
    }
}
//IEpollCallback
STDMETHODIMP CNetworkListen::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(INVALID_FD == m_fd)
        return E_FAIL;

    if(ET_Epoll_Input == type)
    {
        network_stream_param param;
        socklen_t in_len = sizeof(param.addr);
        param.fd = ::accept(m_fd,(sockaddr*)&param.addr,&in_len);
        if(INVALID_FD == param.fd)
        {
            JCHK2(EAGAIN == errno || EWOULDBLOCK == errno,E_FAIL,
                "could not accept error id:%d msg:%s",errno,strerror(errno));
        }
        else
        {
            dom_ptr<IFilter> spFilter;
            JCHK(spFilter.p = static_cast<IFilter*>(m_class->func(IID(IFilter),NULL,false,&param)),E_FAIL);
            //LOG(0,"fd[%d]port:%d connect accept[%d]",m_fd,m_port,param.fd);
            return m_ep->Notify(ET_Listen_Accept,0,spFilter.p);
        }
    }
    return E_AGAIN;
}

CNetworkStream::CNetworkStream()
:m_callback(NULL)
,m_fd(INVALID_FD)
,m_bufRead(NULL)
,m_lenRead(0)
,m_begRead(0)
,m_endRead(0)
,m_flagRead(0)
,m_bufWrite(NULL)
,m_lenWrite(0)
,m_posWrite(0)
,m_indexWrite(0)
,m_offsetWrite(0)
,m_flagWrite(0)
,m_buf(NULL)
,m_flag(READ_FLAG|WRITE_FLAG|ANSY_FLAG)
,m_locker(NULL)
,m_event(false)
,m_thread_read(0)
{
    //ctor
    memset(&m_addr,0,sizeof(m_addr));
    memset(&m_status,0,sizeof(m_status));
}

bool CNetworkStream::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_callback = (ICallback*)pOuter,false);
    if(NULL != pParam)
    {
        network_stream_param* param = static_cast<network_stream_param*>(pParam);
        JCHK(INVALID_FD != param->fd,false);
        m_fd = param->fd;
        m_addr = param->addr;
        m_name = inet_ntoa(m_addr.sin_addr);
        int flags = fcntl(m_fd, F_GETFL, 0);
        JCHK3(0 <= fcntl(m_fd, F_SETFL, flags | O_NONBLOCK),false,
            "set fd:%d nonblock fail,error:%d msg:%s",m_fd,errno,strerror(errno));
        LOG(0,"%s[%p]:[%s] accept connect",
            Class().name,(ICallback*)this,m_name.c_str());
    }

    dom_ptr<IEpoll> spEpoll;
    JCHK(spEpoll.QueryFrom(g_pSite->GetObj()),false);
    JCHK(S_OK == spEpoll->CreatePoint(this,&m_ep),false);
    JCHK(NULL != (m_locker = m_ep->GetLocker()),false);
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
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(uint32_t) CNetworkStream::GetFlag()
{
    return m_flag;
}

STDMETHODIMP CNetworkStream::Open(const char* pUrl,uint32_t mode)
{
    JCHK(NULL != pUrl,E_INVALIDARG);

    HRESULT hr;

    CUrl url;
    JIF(url.Set(pUrl));

    JCHK(false == url.m_host.empty(),E_INVALIDARG);

    Close();

    memset(&m_status,0,sizeof(m_status));

    addrinfo hints;
    memset(&hints,0,sizeof(hints));
    addrinfo *result, *rp;
    sockaddr_in* addr;
    hints.ai_family = AF_INET;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;     /* All interfaces */


    const char* pPort = 0 == url.m_port ? url.m_protocol.c_str() : NULL;
    JCHK4(0 == (hr = getaddrinfo(url.m_host.c_str(), pPort, &hints, &result)),
        E_FAIL,"host:[%s] port:[%s] get local address info fail:%d msg:%s",url.m_host.c_str(),pPort,hr,gai_strerror(hr));

    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
        rp->ai_socktype |= SOCK_NONBLOCK;
        addr = (sockaddr_in*)rp->ai_addr;
        if(0 != url.m_port)
            addr->sin_port = htons(url.m_port);
        m_fd = socket(rp->ai_family, rp->ai_socktype, 0/*rp->ai_protocol*/);
        if(INVALID_FD == m_fd)
            continue;
        JIF(m_ep->Add(m_fd));
        if(0 == connect(m_fd, rp->ai_addr, rp->ai_addrlen) || EINPROGRESS == errno)
        {
            m_name = url.m_host;
            m_addr = *addr;
            //LOG(0,"host:%s:%d[%d] connect",m_name.c_str(),ntohs(addr->sin_port),m_fd);
            break;
        }
        LOG(1,"connect error id:%d msg:%s",errno,strerror(errno));
        JIF(m_ep->Del(m_fd));
        close(m_fd);
        m_fd = INVALID_FD;
    }
    freeaddrinfo(result);
    JCHK(NULL != rp,E_FAIL);

    LOG(0,"%s[%p]:[%s] connect",
        Class().name,(ICallback*)this,m_name.c_str());
    return hr;
}

STDMETHODIMP_(void) CNetworkStream::Close()
{
    CLocker locker(m_locker);

    if(INVALID_FD != m_fd)
    {
        SetEventEnable(false);
        if(NULL != m_bufRead)
        {
            free(m_bufRead);
            m_bufRead = NULL;
        }
        m_lenRead = 0;
        m_begRead = 0;
        m_endRead = 0;
        if(NULL != m_bufWrite)
        {
            free(m_bufWrite);
            m_bufWrite = NULL;
        }
        m_lenWrite = 0;
        m_posWrite = 0;
        m_indexWrite = 0;
        m_offsetWrite = 0;
        m_frame = NULL;
        close(m_fd);
        m_fd = INVALID_FD;
    }
}

STDMETHODIMP CNetworkStream::SetEventEnable(bool enable)
{
    CLocker locker(m_locker);
    JCHK(INVALID_FD != m_fd,E_FAIL);

    if(m_event == enable)
        return S_OK;
    else
    {
        HRESULT hr;
        if(true == enable)
        {
            JIF(m_ep->Add(m_fd));
        }
        else
        {
            JIF(m_ep->Del(m_fd));
        }
        m_event = enable;
        m_thread_read = 0;
        return hr;
    }
}

STDMETHODIMP_(bool) CNetworkStream::GetEventEnable()
{
    CLocker locker(m_locker);
    return m_endRead;
}

STDMETHODIMP CNetworkStream::SetTimer(int id,uint32_t duration,bool one_shoot)
{
    JCHK(m_ep != NULL,E_FAIL);
    return m_ep->SetTimer(id,duration,one_shoot);
}

STDMETHODIMP_(bool) CNetworkStream::CanRead()
{
    CLocker locker(m_locker);
    return m_begRead >= m_lenRead;
}

STDMETHODIMP_(bool) CNetworkStream::CanWrite()
{
    CLocker locker(m_locker);
    return NULL == m_frame && m_posWrite >= m_lenWrite;
}

STDMETHODIMP CNetworkStream::Read(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf)
{
    JCHK(0 < szBuf,E_INVALIDARG);
    CLocker locker(m_locker);

    bool is_peek = 0 != (flag & IStream::READ_FLAG_PEEK);

    if(m_begRead < m_lenRead)
    {
        if(szBuf > m_lenRead - m_begRead)
        {
            uint8_t* buf;
            JCHK(buf = (uint8_t*)malloc(szBuf),E_OUTOFMEMORY);
            memcpy(buf,m_bufRead + m_begRead,m_endRead - m_begRead);
            free(m_bufRead);

            m_bufRead = buf;
            m_lenRead = szBuf;
            m_endRead -= m_begRead;
            m_begRead = 0;
        }
        else
        {
            if(szBuf <= m_endRead - m_begRead)
            {
                if(NULL != pBuf)
                {
                    memcpy(pBuf,m_bufRead + m_begRead , szBuf);
                    if(false == is_peek)
                    {
                        m_begRead += szBuf;
                        m_status.read_total_size += szBuf;
                    }
                }
                else if(NULL != ppBuf)
                {
                    *ppBuf = m_bufRead + m_begRead;
                    if(false == is_peek)
                    {
                        m_begRead += szBuf;
                        m_status.read_total_size += szBuf;
                    }
                }
                return S_OK;
            }
        }
    }
    else
    {
        JCHK(m_bufRead = (uint8_t*)realloc(m_bufRead,szBuf),E_OUTOFMEMORY);

        m_lenRead = szBuf;
        m_endRead = 0;
        m_begRead = 0;
    }

    HRESULT hr = Recv(m_bufRead,m_lenRead,m_endRead,flag);

    if(S_OK > hr)
    {
        if(E_AGAIN == hr)
            m_flagRead = flag;
        return hr;
    }

    if(NULL != pBuf)
    {
        memcpy(pBuf,m_bufRead + m_begRead,szBuf);
        if(false == is_peek)
        {
            m_begRead += szBuf;
            m_status.read_total_size += szBuf;
        }
    }
    return hr;
}

STDMETHODIMP CNetworkStream::Write(const void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf)
{
    JCHK(NULL != pBuf,E_INVALIDARG);

    CLocker locker(m_locker);

    HRESULT hr;

    if(m_frame != NULL || m_posWrite < m_lenWrite)
    {
        m_flag = flag;
        return E_AGAIN;
    }
//    if(0 == m_status.write_total_size)
//    {
//        uint64_t time = m_ep->GetClock();
//        printf("%p write start use :%lu ms\n",this,time - m_time);
//        m_time = time;
//    }
    if(0 != (flag&IStream::WRITE_FLAG_FRAME))
    {
        IMediaFrame* pFrame = (IMediaFrame*)pBuf;
        hr = Send(pFrame,m_indexWrite,m_offsetWrite,flag);
        if(E_AGAIN == hr)
        {
            m_flagWrite = flag;
            m_frame = pFrame;
            hr = 0;
        }
    }
    else
    {
        JCHK(0 < szBuf,E_INVALIDARG);

        uint32_t pos = 0;
        hr = Send((const uint8_t*)pBuf,szBuf,pos,flag);
        if(E_AGAIN == hr)
        {
            szBuf -= pos;
            JCHK(m_bufWrite = (uint8_t*)realloc(m_bufWrite,szBuf),E_OUTOFMEMORY);
            memcpy(m_bufWrite,(const uint8_t*)pBuf+pos,szBuf);
            m_lenWrite = szBuf;
            m_posWrite = 0;
            m_flagWrite = flag;
            hr = 0;
        }
    }
    if(S_OK < hr)
        m_status.write_total_size += szBuf;
    return hr;
}

STDMETHODIMP CNetworkStream::Flush()
{
    return S_OK;
}

STDMETHODIMP_(int64_t) CNetworkStream::GetPos()
{
    JCHK0(false,E_FAIL,"network stream not support ger position");
}

STDMETHODIMP_(int64_t) CNetworkStream::Seek(int64_t position,seek_type type)
{
    JCHK0(false,E_FAIL,"network stream not support seek");
}

STDMETHODIMP_(int64_t) CNetworkStream::GetLength()
{
    return m_endRead - m_begRead;
}

STDMETHODIMP CNetworkStream::SetLength(int64_t len)
{
    return E_FAIL;
}

STDMETHODIMP_(bool) CNetworkStream::IsOpen()
{
    return INVALID_FD != m_fd;
}

//IEventCallback
STDMETHODIMP CNetworkStream::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    HRESULT hr;
    if(ET_EOF == type)
    {
        Close();
    }
    else if(ET_Error == type)
    {
        JCHK(INVALID_FD != m_fd,E_FAIL);
        socklen_t len = sizeof(errno);
        getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &errno, &len);
        param1 = errno;
        LOG(0,"%s[%p]:[%s] socket error id:%d message:%s",
            Class().name,(ICallback*)this,m_name.c_str(),errno,strerror(errno));
    }
    else
    {
        CLocker locker(m_locker);
        if(false == m_event)
            return E_AGAIN;
        switch(type)
        {
            case ET_Epoll_Input:
            {
                if(0 == m_thread_read)
                    m_thread_read = param1;
                else if(m_thread_read != param1)
                    return E_AGAIN;

                JIF(Recv(m_bufRead,m_lenRead,m_endRead,m_flagRead));
                type = ET_Stream_Read;
            }
            break;
            case ET_Epoll_Output:
            {
                if(m_frame != NULL)
                {
                    //LOG(0,"[%p] network stream E_AGAIN write",(IEpollCallback*)this);
                    JIF(Send(m_frame,m_indexWrite,m_offsetWrite,m_flagWrite));
                    m_frame = NULL;
                }
                else
                {
                    JIF(Send(m_bufWrite,m_lenWrite,m_posWrite,m_flagWrite));
                }
                type = ET_Stream_Write;
            }
            break;
        }
    }
    return m_callback->OnEvent((IStream*)this,NULL,type,param1,param2,param3);
}

STDMETHODIMP_(IStream::status&) CNetworkStream::GetStatus()
{
    return m_status;
}

HRESULT CNetworkStream::Recv(uint8_t* pBuf,uint32_t szBuf,uint32_t& pos,uint8_t flag)
{
    JCHK(pos <= szBuf,E_INVALIDARG);
    HRESULT hr = 0;
    while(pos < szBuf)
    {
        void* buf = pBuf+pos;
        uint32_t len = szBuf-pos;
        ssize_t sz;
        do
        {
            sz = read(m_fd,buf,len);

        }while(0 > sz && EINTR == errno);

        if(0 > sz)
        {
            //LOG(0,"network stream read new hold thread:%x clear",m_thread_read);
            if(EAGAIN == errno || EWOULDBLOCK == errno)
            {
                m_thread_read = 0;
                return E_AGAIN;
            }
            else
            {
                LOG(0,"%s[%p]:[%s] read error id:%d message:%s",
                    Class().name,(ICallback*)this,m_name.c_str(),errno,strerror(errno));
                return E_FAIL;
            }
        }
        else if(0 == sz)
        {
            LOG(0,"%s[%p]:[%s] read break",
                Class().name,(ICallback*)this,m_name.c_str());
            Close();
            return E_EOF;
        }
        else
        {
            pos += sz;
            m_status.read_real_size += sz;
            hr += sz;
        }
    }
    return hr;
}

HRESULT CNetworkStream::Send(const uint8_t* pBuf,uint32_t szBuf,uint32_t& pos,uint8_t flag)
{
    JCHK(pos <= szBuf,E_INVALIDARG);
    HRESULT hr = 0;
    while(pos < szBuf)
    {
        const void* buf = pBuf+pos;
        uint32_t len = szBuf-pos;
        ssize_t sz;
        do
        {
            sz = write(m_fd,buf,len);
        }while(0 > sz && EINTR == errno);

        if(0 > sz)
        {
            if(EAGAIN == errno || EWOULDBLOCK == errno)
            {
                return E_AGAIN;
            }
            else
            {
                LOG(0,"%s[%p]:[%s] write error id:%d message:%s",
                    Class().name,(ICallback*)this,m_name.c_str(),errno,strerror(errno));
                return E_FAIL;
            }
        }
        else if(0 == sz)
        {
            LOG(0,"%s[%p]:[%s] write break",
                Class().name,(ICallback*)this,m_name.c_str());
            Close();
            return E_EOF;
        }
        else
        {
            hr += sz;
            pos += sz;
            m_status.write_real_size += sz;
//            IEpollCallback::total_count += sz;
        }
    }
    if(0 != (flag & WRITE_FLAG_EOF))
    {
        Close();
        return E_EOF;
    }
    else
        return hr;
}
const uint32_t OUTPUT_BUF_SIZE = 60;
HRESULT CNetworkStream::Send(IMediaFrame* pFrame,uint32_t& index,uint32_t& offset,uint8_t flag)
{
    HRESULT hr = 0;
    while(true)
    {
        const IMediaFrame::buf* input_buf;
        uint32_t count = OUTPUT_BUF_SIZE,size;
        input_buf = pFrame->GetBuf(index,&count,&size);
        if(NULL == input_buf)
        {
            JCHK(0 == offset,E_FAIL);
            index = 0;
            break;
        }
        IMediaFrame::buf output_buf[OUTPUT_BUF_SIZE];
        memcpy(output_buf,input_buf,sizeof(IMediaFrame::buf)*count);

        if(0 < offset)
        {
            JCHK(offset < output_buf[0].size,E_INVALIDARG);
            output_buf[0].data += offset;
            output_buf[0].size -= offset;
            size -= offset;
        }
        ssize_t sz;
        do
        {
            sz = writev(m_fd,(const iovec*)output_buf,count);

        }while(0 > sz && EINTR == errno);

        if(0 > sz)
        {
            if(EAGAIN == errno || EWOULDBLOCK == errno)
            {
                //LOG(0,"[%p] network stream return E_AGAIN",(IEpollCallback*)this);
//                m_write = 0;
                return E_AGAIN;
            }
            else
            {
                LOG(0,"%s[%p]:[%s] write error id:%d message:%s",
                    Class().name,(ICallback*)this,m_name.c_str(),errno,strerror(errno));
                return E_FAIL;
            }
        }
        else if(0 == sz)
        {
            LOG(0,"%s[%p]:[%s] write break",Class().name,this,m_name.c_str());
            Close();
            return E_EOF;
        }
        else if(sz < size)
        {
            m_status.write_real_size += sz;
            hr += sz;
            uint32_t i = 0;
            while(count > 0)
            {
                if((size_t)sz >= output_buf[i].size)
                {
                    JCHK(0 < count,E_FAIL);

                    sz -= output_buf[i].size;
                    --count;
                    offset = 0;
                    ++i;
                    ++index;
                }
                else
                {
                    offset += (uint32_t)sz;
                    break;
                }
            }
        }
        else
        {
            offset = 0;
            index += count;
            m_status.write_real_size += sz;
            hr += sz;
        }
    }
    if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_EOF) || 0 != (flag & WRITE_FLAG_EOF))
    {
        Close();
        return E_EOF;
    }
    else
        return hr;
}
