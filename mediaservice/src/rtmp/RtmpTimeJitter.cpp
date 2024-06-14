#include "RtmpTimeJitter.hpp"
#include "RtmpGlobal.hpp"
#include "SrsFlvCodec.hpp"

#define BEGIN_THRESHOLD     -1000
#define END_THRESHOLD       1000

RtmpTimeJitter::RtmpTimeJitter()
    : m_type(LmsTimeStamp::middle)
    , m_simple_last_pkt(0)
    , m_simple_last_correct(-1)
    , m_middle_last_pkt(0)
    , m_middle_last_correct(-1)
{

}

RtmpTimeJitter::~RtmpTimeJitter()
{

}

void RtmpTimeJitter::set_correct_type(int type)
{
    m_type = type;
}

int64_t RtmpTimeJitter::correct(IMediaFrame *msg)
{
    int64_t timestamp = 0;

    switch (m_type) {
    case LmsTimeStamp::simple:
        timestamp = simple_correct(msg);
        break;
    case LmsTimeStamp::middle:
        timestamp = middle_correct(msg);
        break;
    default:
        timestamp = msg->info.dts/10000;
        break;
    }

    return timestamp;
}

int64_t RtmpTimeJitter::middle_correct(IMediaFrame *msg)
{
    int64_t timestamp = srs_max(msg->info.dts/10000, 0);

    IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(msg->GetBuf(0));

    if (((msg->info.tag == 9) && SrsFlvVideo::sh((char*)pBuf->data, pBuf->size))
        || ((msg->info.tag == 8) && SrsFlvAudio::sh((char*)pBuf->data, pBuf->size)))
    {
        if (m_middle_last_correct == -1) {
            return 0;
        } else {
            m_middle_last_pkt = timestamp;
            return m_middle_last_correct;
        }
    }

    if (m_middle_last_correct == -1) {
        m_middle_last_correct = m_middle_last_pkt = 0;
        return 0;
    }

    int64_t delta = timestamp - m_middle_last_pkt;

    if (delta < BEGIN_THRESHOLD || delta > END_THRESHOLD) {
        m_middle_last_correct += 10;
    } else {
        m_middle_last_correct += delta;
    }

    m_middle_last_pkt = timestamp;

    return m_middle_last_correct;
}

int64_t RtmpTimeJitter::simple_correct(IMediaFrame *msg)
{
    int64_t timestamp = srs_max(msg->info.dts/10000, 0);

    IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(msg->GetBuf(0));

    if (((msg->info.tag == 9) && SrsFlvVideo::sh((char*)pBuf->data, pBuf->size))
        || ((msg->info.tag == 8) && SrsFlvAudio::sh((char*)pBuf->data, pBuf->size)))
    {
        m_simple_last_pkt = timestamp;
        return timestamp;
    }

    if (m_simple_last_correct != -1) {
        int64_t delta = timestamp - m_simple_last_pkt;
        if (delta < BEGIN_THRESHOLD || delta > END_THRESHOLD) {
            m_simple_last_correct += 10;
        } else {
            m_simple_last_correct = timestamp;
        }
    } else {
        m_simple_last_correct = timestamp;
    }

    m_simple_last_pkt = timestamp;

    return m_simple_last_correct;
}
