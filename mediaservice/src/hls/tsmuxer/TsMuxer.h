/////////////////////////////////////////////////////////////////////////////////////////////
// Project:TsFormat
// Author:
// Date:星期一, 三月 5, 2012
// Description:The Ts Muxer of Dom Frame,Current Only for Video(h.264) and Audio(aac)
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __TSMUXER_H__
#define __TSMUXER_H__


#include "MemoryCache.h"
//#include <cstdint> // c++ 11 header
#include <stdint.h> // c99 header
#include <memory>
#include <limits>
#define VIDEO_STREAM_BASE_ID 0
#define AUDIO_STREAM_BASE_ID 10

#define MAX_PAYLOAD_SIZE (3 * 1024 * 1024)

const unsigned int CTVIT_MPEG2TS_PACKET_SIZE         = 188;
const unsigned int CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE = 184;
const unsigned int CTVIT_MPEG2TS_SYNC_BYTE           = 0x47;
const unsigned int CTVIT_MPEG2TS_PCR_ADAPTATION_SIZE = 6;

const unsigned int CTVIT_PID_PMT   = 0x100;
const unsigned int CTVIT_PID_AUDIO = 0x101;
const unsigned int CTVIT_PID_VIDEO = 0x102;

const unsigned int CTVIT_STREAM_ID_AUDIO = 0xc0;
const unsigned int CTVIT_STREAM_ID_VIDEO = 0xe0;

const unsigned int CTVIT_CC_INDEX_PAT   = 0;
const unsigned int CTVIT_CC_INDEX_PMT   = 1;
const unsigned int CTVIT_CC_INDEX_AUDIO = 2;
const unsigned int CTVIT_CC_INDEX_VIDEO = 3;


//#define _LOG_VIDEO_STREAM

class IMediaFrame
{
public:
    struct MMEDIA_FRAME_INFO
    {
        LONGLONG pts;
        LONGLONG dts;
    };
    MMEDIA_FRAME_INFO info;
    LPBYTE lpdata;
    DWORD dwSize;
    DWORD strmID; // v:0,A:10
};
size_t adts_write_frame_header(uint8_t *buf,size_t size,
                               int obj_type, int sri, int ch,
                               unsigned char* pce, unsigned int pce_size);
size_t annb_write_frame_header(uint8_t *extra_data, int extr_size,
                               uint8_t *ibuf, int isize,
                               uint8_t *obuf, int osize);
/**
 * @brief TS格式复用器
 * 当前仅支持AVC/HEVC+AAC/mp3
 * 不支持多节目多流
 * 支持单音，单视
 * 本类不进行时间戳转换。外部的时间戳单位需为90KHz
 */
class CTsMuxer
{
bool _is_mp3;
bool _is_265;
public:
    //CTsMuxer class declare
    CTsMuxer(bool is_mp3 = false, bool is_265 = false);
    virtual ~CTsMuxer();

    /**
     * @brief OnWriteFrame
     * @param pFrame
     * @param force_Pat_pmt
     * @return
     */
    HRESULT OnWriteFrame(/*CMediaMuxerStream* pStream,*/IMediaFrame* pFrame, bool force_Pat_pmt = false);
    //CTsMuxer
    HRESULT InternalOpen(muxer_sink* s, bool hasV = true, bool hasA = true);
    HRESULT InternalClose();
    void flush()    {        this->m_MemoryCache.Flush();    }

private:
    static BYTE StuffingBytes[CTVIT_MPEG2TS_PACKET_SIZE];
    static DWORD const CRC_Table[256];
    DWORD ComputeCRC(const BYTE* lpData,DWORD dwSize);
protected:
    bool WriteTsHeader(DWORD dwPid,DWORD dwCCIndex,bool bPayloadStart,DWORD& dwPayloadSize,bool bWithPCR,UINT64 nPCR);
    bool WritePAT();
    bool WritePMT(DWORD dwVideoID,DWORD dwAudioID);
    bool WriteVideoPacket(LPBYTE lpData, DWORD dwSize,LONGLONG llDts,LONGLONG llPts);
    bool WriteAudioPacket(LPBYTE lpData, DWORD dwSize,LONGLONG llDts,LONGLONG llPts);
private:
    bool WritePESPacket(DWORD dwPid,DWORD dwCCIndex,DWORD dwStreamID,LPBYTE lpData,DWORD dwSize,LONGLONG llDts,bool bWithDts,LONGLONG llPts,bool bWithPCR);


#ifdef _LOG_VIDEO_STREAM
    HANDLE m_hVideoFile;
    HANDLE m_hAudioFile;
#endif

private:
    CMemoryCache m_MemoryCache;
    bool m_bHasVideo;
    bool m_bHasAudio;
    unsigned int m_nContinuityCounter[4];
    double m_nTimeBase;
    LPBYTE m_lpVideoBuffer;
    LONGLONG m_llLastDts;
    DWORD m_dwAudioBaseIndex;
    DWORD m_dwVideoBaseIndex;
private:
    bool m_bVideoFirst;
    bool m_bAudioFirst;
};

class calc_timestamp
{
    enum AVRounding {
        AV_ROUND_ZERO     = 0, ///< Round toward zero.
        AV_ROUND_INF      = 1, ///< Round away from zero.
        AV_ROUND_DOWN     = 2, ///< Round toward -infinity.
        AV_ROUND_UP       = 3, ///< Round toward +infinity.
        AV_ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
        AV_ROUND_PASS_MINMAX = 8192, ///< Flag to pass INT64_MIN/MAX through instead of rescaling, this avoids special cases for AV_NOPTS_VALUE
    };

public:
    typedef struct AVRational{
        int num; ///< numerator
        int den; ///< denominator
    } AVRational;
    calc_timestamp(AVRational  in_time_base, AVRational out_time_base)
    {
        _in_timebase = in_time_base;
        _out_timebase = out_time_base;
    }

    int64_t calc(uint64_t tm_)
    {
        int64_t b = _in_timebase.num * (int64_t)_out_timebase.den;
        int64_t c = _out_timebase.num * (int64_t)_in_timebase.den;
        AVRounding rnd = (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        return av_rescale_rnd(tm_, b, c, rnd);
    }
private:
#define INT_MAX 0x7FFFFFFF
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
    int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd)
    {
        int64_t r = 0;
        //        av_assert2(c > 0);
        //        av_assert2(b >=0);
        //        av_assert2((unsigned)(rnd&~AV_ROUND_PASS_MINMAX)<=5 && (rnd&~AV_ROUND_PASS_MINMAX)!=4);

        if (c <= 0 || b < 0 || !((unsigned)(rnd&~AV_ROUND_PASS_MINMAX)<=5 && (rnd&~AV_ROUND_PASS_MINMAX)!=4))
            return INT64_MIN;

        if (rnd & AV_ROUND_PASS_MINMAX) {
            if (a == INT64_MIN || a == INT64_MAX)
                return a;
            rnd = (AVRounding)(rnd - AV_ROUND_PASS_MINMAX);
        }

        if (a < 0)
            return -(uint64_t)av_rescale_rnd(-FFMAX(a, -INT64_MAX), b, c, (AVRounding)(rnd ^ ((rnd >> 1) & 1)));

        if (rnd == AV_ROUND_NEAR_INF)
            r = c / 2;
        else if (rnd & 1)
            r = c - 1;

        if (b <= INT_MAX && c <= INT_MAX) {
            if (a <= INT_MAX)
                return (a * b + r) / c;
            else {
                int64_t ad = a / c;
                int64_t a2 = (a % c * b + r) / c;
                if (ad >= INT32_MAX && b && ad > (INT64_MAX - a2) / b)
                    return INT64_MIN;
                return ad * b + a2;
            }
        } else {
#if 1
            uint64_t a0  = a & 0xFFFFFFFF;
            uint64_t a1  = a >> 32;
            uint64_t b0  = b & 0xFFFFFFFF;
            uint64_t b1  = b >> 32;
            uint64_t t1  = a0 * b1 + a1 * b0;
            uint64_t t1a = t1 << 32;
            int i;

            a0  = a0 * b0 + t1a;
            a1  = a1 * b1 + (t1 >> 32) + (a0 < t1a);
            a0 += r;
            a1 += a0 < r;

            for (i = 63; i >= 0; i--) {
                a1 += a1 + ((a0 >> i) & 1);
                t1 += t1;
                if (c <= a1) {
                    a1 -= c;
                    t1++;
                }
            }
            if (t1 > INT64_MAX)
                return INT64_MIN;
            return t1;
        }
#else
            AVInteger ai;
            ai = av_mul_i(av_int2i(a), av_int2i(b));
            ai = av_add_i(ai, av_int2i(r));

            return av_i2int(av_div_i(ai, av_int2i(c)));
        }
#endif
    }
    AVRational _in_timebase;
    AVRational _out_timebase;
};

#endif//__TSMUXER_H__
