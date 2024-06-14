#ifndef LOCKER_H
#define LOCKER_H
#include <pthread.h>

class CLocker
{
    public:
        CLocker();
        CLocker(CLocker& locker);
        CLocker(CLocker* pLocker);
        virtual ~CLocker();
        bool IsValid();
        bool Lock();
        bool TryLock();
        bool Unlock();
    protected:
        bool m_isValid;
        pthread_mutex_t m_mutext;
        CLocker* m_pLocker;
};

#endif // LOCKER_H
