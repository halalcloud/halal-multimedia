#include <time.h>
#include <stdlib.h>
#include <time_expend.h>

int64_t GetTickCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 10000000 + ts.tv_nsec / 100;
}

const int64_t NONE_TIMESTAMP  = 0x8000000000000001;                         //None Timestamp

time_expend::time_expend()
:m_clock(NONE_TIMESTAMP)
,m_begin(NONE_TIMESTAMP)
,m_length(0)
,m_te(NULL)
{

}

time_expend::time_expend(time_expend& te)
:m_clock(NONE_TIMESTAMP)
,m_begin(NONE_TIMESTAMP)
,m_length(0)
,m_te(&te)
{
    m_te->m_begin = GetTickCount();
}

time_expend::~time_expend()
{
    if(NULL != m_te)
    {
        m_te->m_length += GetTickCount() - m_te->m_begin;
        m_te->m_begin = NONE_TIMESTAMP;
    }
}

int64_t time_expend::get_begin()
{
    return NULL == m_te ? m_begin : m_te->get_begin();
}

double time_expend::get()
{
    int64_t now = GetTickCount();
    if(0 > m_clock)
    {
        m_clock = now;
        return 0;
    }
    else
    {
        int64_t duration = now - m_clock;
        int64_t expend = 0;
        if(0 < m_begin)
        {
            expend = now - m_begin;
            m_begin = now;
            if(m_begin < m_clock)
                expend -= m_clock - m_begin;
        }
        expend += m_length;
        m_length = 0;
        m_clock = now;
        return expend * 100.0 / duration;
    }
}
