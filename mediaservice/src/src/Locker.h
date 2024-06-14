#ifndef LOCKER_H
#define LOCKER_H
#include <stdint.h>
#include <pthread.h>
#include <iStream.h>
class CLocker : public ILocker
{
    public:
        CLocker();
        CLocker(CLocker& locker);
        CLocker(ILocker* pLocker);
        virtual ~CLocker();
        STDMETHODIMP Lock();
        STDMETHODIMP Unlock();
        STDMETHODIMP_(bool) TryLock();
    protected:
        pthread_mutex_t m_mutext;
        pthread_mutexattr_t m_mattr;
        ILocker* m_pLocker;

};

#endif // LOCKER_H
