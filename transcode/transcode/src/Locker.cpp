#include <Locker.h>

CLocker::CLocker()
:m_isValid(false)
,m_pLocker(NULL)
{
    m_isValid = 0 == pthread_mutex_init(&m_mutext, NULL);
}

CLocker::CLocker(CLocker& locker)
:m_isValid(false)
,m_pLocker(NULL)
{
    locker.Lock();
    m_pLocker = &locker;
}

CLocker::CLocker(CLocker* pLocker)
:m_isValid(false)
,m_pLocker(NULL)
{
    if(NULL == pLocker)
    {
        m_isValid = 0 == pthread_mutex_init(&m_mutext, NULL);
    }
    else
    {
        pLocker->Lock();
        m_pLocker = pLocker;
    }
}

CLocker::~CLocker()
{
    if(NULL != m_pLocker)
        m_pLocker->Unlock();
    else if(true == m_isValid)
    {
        pthread_mutex_destroy(&m_mutext);
    }
}

bool CLocker::IsValid()
{
    return NULL == m_pLocker ? m_isValid : m_pLocker->IsValid();
}

bool CLocker::Lock()
{
    if(NULL == m_pLocker)
        return true == m_isValid ? 0 == pthread_mutex_lock(&m_mutext) : false;
    else
        return m_pLocker->Lock();
}

bool CLocker::TryLock()
{
    if(NULL == m_pLocker)
        return true == m_isValid ? 0 == pthread_mutex_trylock(&m_mutext) : false;
    else
        return m_pLocker->TryLock();
}

bool CLocker::Unlock()
{
    if(NULL == m_pLocker)
        return true == m_isValid ? 0 == pthread_mutex_unlock(&m_mutext) : false;
    else
        return m_pLocker->Unlock();
}
