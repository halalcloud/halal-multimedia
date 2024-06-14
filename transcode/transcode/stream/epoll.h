#ifndef EPOLL_H
#define EPOLL_H

#include "stdafx.h"
const int EVENT_COUNT = 10;
class CEpoll : public IEpoll
{
    typedef list< pthread_t > ThreadSet;
    typedef ThreadSet::iterator ThreadIt;
public:
    DOM_DECLARE(CEpoll)
    STDMETHODIMP SetTimeout(IEpollCallback* pCallback,int ms);
    STDMETHODIMP Add(int fd,IEpollCallback* pCallback);
    STDMETHODIMP Del(int fd,IEpollCallback* pCallback);
protected:
    static void* Process(void* pParam);
    HRESULT Process();
protected:
    int m_fd;
    int m_timeout;
    ThreadSet m_threads;
    IEpollCallback* m_callback;
    void* m_tag;
    epoll_event m_events[EVENT_COUNT];
    int m_size;
};

#endif // EPOLL_H
