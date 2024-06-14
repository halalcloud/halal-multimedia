#ifndef EPOLL_H
#define EPOLL_H

#include "stdafx.h"
class CEpollPoint;
struct timer
{
    typedef map< int,unique_ptr<timer> > Set;
    typedef Set::iterator It;
    typedef pair<Set::key_type,Set::mapped_type> Pair;

    It _it;
    CEpollPoint* _point;
    int _id;
    uint32_t _duration;
    uint64_t _start;
    uint64_t _clock;
    bool _one_shoot;
    timer(CEpollPoint* point,int id,uint32_t duration,bool one_shoot)
    :_point(point),_id(id),_duration(duration),_start(0),_clock(0),_one_shoot(one_shoot){}
    HRESULT Notify();
};

class CEpoll;
class CEpollPoint : public IEpollPoint
{
public:
    typedef list< dom_ptr<IEpollPoint> > PointSet;
    typedef PointSet::iterator PointIt;
    DOM_DECLARE(CEpollPoint);
    STDMETHODIMP_(ILocker*) GetLocker();
    STDMETHODIMP Add(int fd);
    STDMETHODIMP Del(int fd);
    STDMETHODIMP_(uint32_t) GetCount();
    STDMETHODIMP SetTimer(int32_t id,uint32_t duration,bool one_shoot);
    STDMETHODIMP_(uint64_t) GetClock();
    STDMETHODIMP_(void) SetTag(void* tag);
    STDMETHODIMP_(void*) GetTag();
    HRESULT Notify(uint32_t type,int32_t param1 = 0,void* param2 = NULL,void* param3 = NULL);
    void Release(timer::It it);
public:
    PointIt m_it;
    CEpoll* m_epoll;
    uint32_t m_count;
    ICallback* m_callback;
    timer::Set m_timers;
    CLocker m_locker;
    void* m_tag;
};

class CEpoll : public IEpoll
{
    typedef multimap< uint64_t,timer* > TimerSet;
    typedef TimerSet::iterator TimerIt;
    typedef pair<TimerSet::key_type,TimerSet::mapped_type> TimerPair;

    typedef list< pthread_t > ThreadSet;
    typedef ThreadSet::iterator ThreadIt;

public:
    DOM_DECLARE(CEpoll)
    //IEpoll
    STDMETHODIMP CreatePoint(ICallback* callback,IEpollPoint** ppEP);

    HRESULT Add(int fd,CEpollPoint* pEP);
    HRESULT Del(int fd);
    void Release(CEpollPoint::PointIt it);
    static void* Process(void* pParam);
    HRESULT Process();
    HRESULT Push(timer* pTimer);
    timer* Pop();
protected:
    int m_fd;
    uint32_t m_wait;
    CEpollPoint::PointSet m_points;
    TimerSet m_timers;
    ThreadSet m_threads;
    int32_t m_max_thread;
    int32_t m_count_fd;
    int32_t m_count_thread;
    int32_t m_active_fd;
    int32_t m_active_thread;
    CLocker m_locker;
};

#endif // EPOLL_H
