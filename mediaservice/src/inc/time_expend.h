#ifndef TIME_EXPEND_H_INCLUDED
#define TIME_EXPEND_H_INCLUDED

int64_t GetTickCount();

class time_expend
{
public:
    time_expend();
    time_expend(time_expend& te);
    int64_t get_begin();
    ~time_expend();
    double get();
protected:
    int64_t m_clock;
    int64_t m_begin;
    int64_t m_length;
    time_expend* m_te;
};

#endif // TIME_EXPEND_H_INCLUDED
