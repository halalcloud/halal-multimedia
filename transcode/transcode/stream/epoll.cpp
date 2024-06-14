#include "epoll.h"
#include <auto_ptr.h>
#include <signal.h>

CEpoll::CEpoll()
:m_fd(INVALID_FD)
,m_timeout(EPOLL_DEFAULT_TIME_OUT)
,m_callback(NULL)
,m_tag(NULL)
{
}

bool CEpoll::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK2(INVALID_FD != (m_fd = epoll_create1(0)),false,
        "epoll create fail,error id:%d message:%s",errno,strerror(errno));
    memset(m_events,0,sizeof(m_events));
    m_size = 0;
    return true;
}

bool CEpoll::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        if(INVALID_FD != m_fd)
        {
            close(m_fd);
            m_fd = INVALID_FD;
            kill(getpid(),SIGINT);
            for(ThreadIt it = m_threads.begin() ; it != m_threads.end() ; ++it)
                pthread_join(*it,NULL);
        }
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CEpoll)
DOM_QUERY_IMPLEMENT(IEpoll)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CEpoll::SetTimeout(IEpollCallback* pCallback,int ms)
{
    JCHK(ms >= EPOLL_DEFAULT_TIME_OUT && 0 != ms,E_INVALIDARG);
    m_callback = pCallback;
    m_timeout = ms;
    return S_OK;
}

STDMETHODIMP CEpoll::Add(int fd,IEpollCallback* pCallback)
{
    JCHK(INVALID_FD != fd,E_INVALIDARG);
    JCHK(NULL != pCallback,E_INVALIDARG);

    epoll_event event;
    memset(&event,0,sizeof(event));
    event.data.ptr = pCallback;
    event.events = EPOLLIN | EPOLLOUT | EPOLLOUT | EPOLLET;
    JCHK2(-1 != (epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event)),E_FAIL,
        "epoll add event fail,error id:%d message:%s",errno,strerror(errno));

    if(true == m_threads.empty())
    {
        pthread_t thread = 0;
        JCHK(0 == pthread_create(&thread, NULL, Process, (void*)this),E_FAIL);
        m_threads.push_back(thread);
    }

    return S_OK;
}

STDMETHODIMP CEpoll::Del(int fd,IEpollCallback* pCallback)
{
    JCHK(INVALID_FD != fd,E_INVALIDARG);
    JCHK(NULL != pCallback,E_INVALIDARG);
    JCHK(INVALID_FD != m_fd,E_FAIL);
    JCHK2(-1 != (epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, NULL)),E_FAIL,
        "epoll add event fail,error id:%d message:%s",errno,strerror(errno));
    if(0 < m_size)
    {
        for(uint32_t i=0 ; i<EVENT_COUNT ; ++i)
        {
            if(m_events[i].data.ptr == pCallback)
            {
                m_events[i].data.ptr = NULL;
                --m_size;
                break;
            }
        }
    }
    return S_OK;
}

void* CEpoll::Process(void* pParam)
{
    CEpoll* pThis = (CEpoll*)pParam;
    pThis->Process();
    return NULL;
}

HRESULT CEpoll::Process()
{
    int count = 0;
    printf("work thread:%lu enter\n",pthread_self());
    int64_t start = GetTickCount();
    while(INVALID_FD != m_fd)
    {
        memset(m_events,0,sizeof(m_events));
        while(0 < (count = epoll_wait(m_fd,m_events,EVENT_COUNT,m_timeout)))
        {
            m_size = count;
            do
            {
                for(int i=0 ; i<count ; ++i)
                {
                    IEpollCallback* pCallback;
                    if(NULL != (pCallback = (IEpollCallback*)m_events[i].data.ptr))
                    {
                        if(IS_FAIL(pCallback->OnEvent(m_events[i].events)))
                        {
                            m_events[i].data.ptr = NULL;
                            --m_size;
                        }
                    }
                }
                if(m_callback != NULL && 0 < m_timeout)
                {
                    int64_t stop = GetTickCount();
                    if(stop - start >= m_timeout * 10000)
                    {
                        start = stop;
                        //LOG(0,"timer event in");
                        m_callback->OnEvent(et_timeout,m_tag);
                        //LOG(0,"timer event out");
                    }
                }
            }while(0 < m_size);
        }
        if(0 == count)
        {
            if(m_callback != NULL)
            {
                if(EPOLL_DEFAULT_TIME_OUT != m_timeout)
                {
                    start = GetTickCount();
                    //LOG(0,"timer event in");
                    m_callback->OnEvent(et_timeout,m_tag);
                    //LOG(0,"timer event out");
                }
            }
        }
        else
        {
            if(EINTR != errno)
                break;
        }
    }
    if(INVALID_FD != m_fd)
        printf("work thread:%lu exit,error id:%d message:%s\n",pthread_self(),errno,strerror(errno));
    else
        printf("work thread:%lu exit\n",pthread_self());
    return count;
}
