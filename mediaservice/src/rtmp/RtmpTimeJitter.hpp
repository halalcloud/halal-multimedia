#ifndef RTMPTIMEJITTER_HPP
#define RTMPTIMEJITTER_HPP

#include "stdafx.h"

namespace LmsTimeStamp {
    enum CorrectType { simple = 0, middle };
}

class RtmpTimeJitter
{
public:
    RtmpTimeJitter();
    ~RtmpTimeJitter();

    void set_correct_type(int type);

    int64_t correct(IMediaFrame *msg);

private:
    int64_t simple_correct(IMediaFrame *msg);
    int64_t middle_correct(IMediaFrame *msg);

private:
    int m_type;

    int64_t m_simple_last_pkt;
    int64_t m_simple_last_correct;

private:
    int64_t m_middle_last_pkt;
    int64_t m_middle_last_correct;

};

#endif // RTMPTIMEJITTER_HPP
