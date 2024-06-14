#include "epoll.h"

HRESULT timer::Notify()
{
    HRESULT hr = E_EOF;
    if(0 == _duration || S_OK > (hr = _point->Notify(ET_Epoll_Timer,_id,&_start,&_clock)) || true == _one_shoot)
    {
        _point->Release(_it);
        if(S_OK <= hr)
            hr = E_EOF;
    }
    return hr;
}

CEpollPoint::CEpollPoint()
:m_epoll(NULL)
,m_count(0)
,m_callback(NULL)
,m_tag(NULL)
{

}

bool CEpollPoint::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_epoll = (CEpoll*)pOuter,false);
    JCHK(m_callback = (ICallback*)pParam,false);
    return true;
}

bool CEpollPoint::FinalDestructor(bool finally)
{
    if(false == finally)
    {
        CLocker locker(m_locker);
        m_callback = NULL;
        if(0 == m_count&& true == m_timers.empty())
            SetTimer(0,0,true);
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CEpollPoint)
DOM_QUERY_IMPLEMENT(IEpollPoint)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(ILocker*) CEpollPoint::GetLocker()
{
    return &m_locker;
}

STDMETHODIMP CEpollPoint::Add(int fd)
{
    HRESULT hr;
    CLocker locker(m_locker);
    JIF(m_epoll->Add(fd,this));
    ++m_count;
    return hr;
}

STDMETHODIMP CEpollPoint::Del(int fd)
{
    HRESULT hr;
    CLocker locker(m_locker);
    JIF(m_epoll->Del(fd));
    --m_count;
    return hr;
}

STDMETHODIMP_(uint32_t) CEpollPoint::GetCount()
{
    CLocker locker(m_locker);
    return m_count;
}

STDMETHODIMP CEpollPoint::SetTimer(int32_t id,uint32_t duration,bool one_shoot)
{
    CLocker locker(m_locker);
    timer::It it = m_timers.find(id);
    if(it == m_timers.end())
    {
        timer* pTimer;
        JCHK(pTimer = new timer(this,id,duration,one_shoot),E_OUTOFMEMORY);
        pTimer->_it = m_timers.insert(timer::Pair(id,unique_ptr<timer>(pTimer))).first;
        return m_epoll->Push(pTimer);
    }
    else
    {
        it->second->_duration = duration;
    }
    return S_OK;
}

STDMETHODIMP_(uint64_t) CEpollPoint::GetClock()
{
    return GetMSCount();
}

STDMETHODIMP_(void) CEpollPoint::SetTag(void* tag)
{
    m_tag = tag;
}

STDMETHODIMP_(void*) CEpollPoint::GetTag()
{
    return m_tag;
}

HRESULT CEpollPoint::Notify(uint32_t type,int32_t param1,void* param2,void* param3)
{
    dom_ptr<ICallback> spCallback;
    {
        CLocker locker(m_locker);
        if(0 == m_count || NULL == m_callback)
            return E_EOF;
        else
            spCallback = m_callback;
    }
    HRESULT hr = spCallback->OnEvent(this,NULL,type,param1,param2,param3);
    if(S_OK > hr && E_AGAIN != hr)
        spCallback->OnEvent(this,NULL,ET_EOF,hr);
    return hr;
}

void CEpollPoint::Release(timer::It it)
{
    {
        CLocker locker(m_locker);
        m_timers.erase(it);
        if(false == m_timers.empty() || NULL != m_callback)
            return;
    }
    m_epoll->Release(m_it);
}


const uint32_t DEFAULT_SAMPLE_DURATION = 100;

CEpoll::CEpoll()
:m_fd(INVALID_FD)
,m_wait(DEFAULT_SAMPLE_DURATION)
,m_max_thread(4)
,m_count_fd(0)
,m_count_thread(0)
,m_active_fd(0)
,m_active_thread(0)
{
}

bool CEpoll::FinalConstruct(Interface* pOuter,void* pParam)
{
    CLocker locker(m_locker);

    JCHK2(INVALID_FD != (m_fd = epoll_create1(0)),false,
        "epoll create fail,error id:%d message:%s",errno,strerror(errno));

    pthread_t thread = 0;
    JCHK(0 == pthread_create(&thread, NULL, Process, (void*)this),E_FAIL);
    m_threads.push_back(thread);

    return true;
}

bool CEpoll::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        if(INVALID_FD != m_fd)
        {
            int fd = m_fd;
            m_fd = INVALID_FD;
            close(fd);
            m_wait = 0;
            for(ThreadIt it = m_threads.begin() ; it != m_threads.end() ; ++it)
            {
                pthread_join(*it,NULL);
            }
        }
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CEpoll)
DOM_QUERY_IMPLEMENT(IEpoll)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CEpoll::CreatePoint(ICallback* callback,IEpollPoint** ppEP)
{
    JCHK(NULL != callback,E_INVALIDARG);
    JCHK(NULL != ppEP,E_INVALIDARG);

    CLocker locker(m_locker);

    bool br;
    interface_imp<CEpollPoint>* pPoint;
    JCHK(NULL != (pPoint = new interface_imp<CEpollPoint>(this,false,callback,br)),E_OUTOFMEMORY);
    JCHK(true == br,E_FAIL);

    dom_ptr<IEpollPoint> spPoint;
    JCHK(spPoint.p = (IEpollPoint*)pPoint->QueryInterface(IID(IEpollPoint)),E_FAIL);

    pPoint->m_it = m_points.insert(m_points.end(),spPoint);

    return spPoint.CopyTo(ppEP);
}

HRESULT CEpoll::Add(int fd,CEpollPoint* pEP)
{
    epoll_event event;
    memset(&event,0,sizeof(event));
    event.data.ptr = pEP;
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;

    JCHK2(-1 != (epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event)),E_FAIL,
        "epoll add event fail,error id:%d message:%s",errno,strerror(errno));

    int32_t count_thread = m_threads.size();
    if(++m_count_fd > count_thread && count_thread < m_max_thread)
    {
        pthread_t thread = 0;
        JCHK(0 == pthread_create(&thread, NULL, Process, (void*)this),E_FAIL);
        m_threads.push_back(thread);
    }
    return S_OK;
}

HRESULT CEpoll::Del(int fd)
{
    JCHK(INVALID_FD != fd,E_INVALIDARG);
    JCHK2(-1 != (epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, NULL)),E_FAIL,
        "epoll del event fail,error id:%d message:%s",errno,strerror(errno));

    --m_count_fd;
    //LOG(0,"CEpoll::Release count:%d",m_count_fd);
    return S_OK;
}

void CEpoll::Release(CEpollPoint::PointIt it)
{
    CLocker locker(m_locker);
    m_points.erase(it);
}

void* CEpoll::Process(void* pParam)
{
    CEpoll* pThis = (CEpoll*)pParam;
    pThis->Process();
    return NULL;
}

const int32_t WAIT_MAX_FD_COUNT = 1024;

HRESULT CEpoll::Process()
{
    HRESULT hr = S_OK;

    ++m_count_thread;

    pthread_t thread = pthread_self();

    epoll_event events[WAIT_MAX_FD_COUNT];

    while(INVALID_FD != m_fd)
    {
        int32_t fd_count = m_count_fd - m_active_fd;
        if(0 > fd_count)
            fd_count = 0;

        int32_t thread_count = m_count_thread - m_active_thread;
        if(1 > thread_count)
            thread_count = 1;

        int32_t wait_count = fd_count / thread_count;
        if(0 != (fd_count % thread_count))
            wait_count += 1;
        else if(0 == wait_count)
        {
            wait_count = m_count_fd / m_count_thread;
            if(0 != (m_count_fd % m_count_thread))
                wait_count += 1;
            else if(0 == wait_count)
                wait_count = 1;
        }
        else if(wait_count > WAIT_MAX_FD_COUNT)
            wait_count = WAIT_MAX_FD_COUNT;

//        printf("thread total:%d active:%d,fd total:%d active:%d,wait fd count:%d\n",
//            m_count_thread,m_active_thread,m_count_fd,m_active_fd,wait_count);
        int size = epoll_wait(m_fd,events,(int)wait_count,m_wait);
        if(0 < size)
        {
            ++m_active_thread;
            int32_t active_fd = 0;
            m_active_fd += size;
            active_fd += size;
            while(0 < active_fd)
            {
                for(int i=0 ; i<size ; ++i)
                {
                    CEpollPoint* ep;
                    if(NULL != (ep = (CEpollPoint*)events[i].data.ptr))
                    {
                        if(0 != (events[i].events & (EPOLLERR)))
                        {
                            ep->Notify(ET_Error,thread);
                            events[i].events = 0;
                        }
                        else if(0 != (events[i].events & EPOLLIN))
                        {
                            hr = ep->Notify(ET_Epoll_Input,thread);
                            if(S_OK > hr)
                            {
                                if(E_AGAIN == hr)
                                    events[i].events &= ~EPOLLIN;
                                else
                                    events[i].events = 0;
                            }
                        }
                        else if(0 != (events[i].events & EPOLLOUT))
                        {
                            hr = ep->Notify(ET_Epoll_Output,thread);
                            if(S_OK > hr)
                            {
                                if(E_AGAIN == hr)
                                    events[i].events &= ~EPOLLOUT;
                                else
                                    events[i].events = 0;
                            }
                        }
                        else
                            events[i].events = 0;

                        if(0 == events[i].events)
                        {
                            events[i].data.ptr = NULL;
                            --m_active_fd;
                            --active_fd;
                        }
                    }
                }
            }
            --m_active_thread;
        }
        else if(0 == size)
        {
            if(INVALID_FD  == m_fd)
                break;

            timer* pTimer;
            while(NULL != (pTimer = Pop()))
            {
                if(S_OK <= pTimer->Notify())
                {
                    JIF(Push(pTimer));
                }
            }
        }
        else if(EINTR != errno)
        {
            LOG(0,"epoll wait fail,error id:%d message:%s",errno,strerror(errno));
            break;
        }
    }
    --m_count_thread;
    return hr;
}

HRESULT CEpoll::Push(timer* pTimer)
{
    JCHK(NULL != pTimer,E_INVALIDARG);
    CLocker locker(m_locker);
    if(0 == pTimer->_duration)
        pTimer->_clock = GetMSCount() + m_wait;
    else if(true == pTimer->_one_shoot)
        pTimer->_clock = GetMSCount() + pTimer->_duration;
    else if(0 < pTimer->_clock)
        pTimer->_clock += pTimer->_duration;
    m_timers.insert(TimerPair(pTimer->_clock,pTimer));
    return S_OK;
}

timer* CEpoll::Pop()
{
    CLocker locker(m_locker);
    timer* pTimer = NULL;
    TimerIt it = m_timers.begin();
    if(it != m_timers.end())
    {
        uint64_t clock = GetMSCount();
        if(clock >= it->first)
        {
            pTimer = it->second;
            m_timers.erase(it);
            if(0 == pTimer->_clock)
            {
                pTimer->_clock = clock;
                pTimer->_start = clock;
            }
        }
    }
    return pTimer;
}
