#include "Locker.h"
#include <dump.h>
CLocker::CLocker()
:m_pLocker(NULL)
{
    LCHK(0 == pthread_mutexattr_init(&m_mattr));
    LCHK(0 == pthread_mutexattr_settype(&m_mattr,PTHREAD_MUTEX_RECURSIVE));
    LCHK(0 == pthread_mutex_init(&m_mutext, &m_mattr));
}

CLocker::CLocker(CLocker& locker)
:m_pLocker(&locker)
{
    m_pLocker->Lock();
}

CLocker::CLocker(ILocker* pLocker)
:m_pLocker(pLocker)
{
    if(NULL == m_pLocker)
    {
        LCHK(0 == pthread_mutexattr_init(&m_mattr));
        LCHK(0 == pthread_mutexattr_settype(&m_mattr,PTHREAD_MUTEX_RECURSIVE));
        LCHK(0 == pthread_mutex_init(&m_mutext, &m_mattr));
    }
    else
    {
        m_pLocker->Lock();
    }
}

CLocker::~CLocker()
{
    if(NULL != m_pLocker)
        m_pLocker->Unlock();
    else
    {
        pthread_mutex_destroy(&m_mutext);
        pthread_mutexattr_destroy(&m_mattr);
    }
}

STDMETHODIMP CLocker::Lock()
{
    if(NULL != m_pLocker)
        return m_pLocker->Lock();
    else
        return 0 == pthread_mutex_lock(&m_mutext) ? S_OK : E_FAIL;
}

STDMETHODIMP CLocker::Unlock()
{
    if(NULL != m_pLocker)
        return m_pLocker->Unlock();
    else
        return 0 == pthread_mutex_unlock(&m_mutext) ? S_OK : E_FAIL;

}

STDMETHODIMP_(bool) CLocker::TryLock()
{
    return 0 == pthread_mutex_trylock(&m_mutext);
}
