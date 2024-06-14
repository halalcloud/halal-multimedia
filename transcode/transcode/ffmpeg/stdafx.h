#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

extern "C"
{
#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#endif

#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/hevc.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/internal.h>
#include <libavcodec/mpeg4audio.h>
#include <libavcodec/get_bits.h>
#include <libavcodec/put_bits.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}
#include <unistd.h>
#include <string>
#include <dom.h>
#include <iMulitimedia.h>
#include <time_expend.h>


using namespace std;

const AVRational FRAME_TIMEBASE = {1,10000000};
const AVRational VIDEO_TIMEBASE = {1,10000};
const int VIDEO_ALIGN = 16;
const int AUDIO_ALIGN = 4;
int gcd(int a,int b);

HRESULT AVPacketToFrame(IMediaFrame* pFrame,AVPacket& packet,const AVRational& base,int frame_size);
HRESULT FrameToAVPacket(AVPacket* pPacket,IMediaFrame* pFrame,const AVRational* base = NULL);

HRESULT AVFrameToFrame(IMediaType* pMT,IMediaFrame* pDest,AVFrame* pSour);
HRESULT FrameToAVFrame(IMediaType* pMT,AVFrame* pDest,IMediaFrame* pSour,AVCodecContext* ctxCodec);

void GetAudioSampleRate(const int* supported_samplerates,int& sample_rate);
bool GetVideoDuration(const AVRational* supported_framerates,int64_t& duration);
void GetOption(void* obj,IProfile* pProfile);

bool SetFilterInfo(IFilter* pFilter,int64_t* start_time,int64_t* length,int64_t* bitrate);

void get_pps_sps(uint8_t* buf, uint32_t size, bool is264, uint32_t& offset, uint32_t& len);

#endif // STDAFX_H_INCLUDED
