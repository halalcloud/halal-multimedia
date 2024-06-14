// The functions contained in this file are pretty dummy
// and are included only as a placeholder. Nevertheless,
// they *will* get included in the shared library if you
// don't remove them :)
//
// Obviously, you 'll have to write yourself the super-duper
// functions to include in the resulting library...
// Also, it's not necessary to write every function in this file.
// Feel free to add more files in this project. They will be
// included in the resulting library.
#include "stdafx.h"
#include <stdlib.h>
#include "MediaType.h"
#include "FFmpegDemuxer.h"
#include "FFmpegMuxer.h"
#include "FFmpegAudioDecoder.h"
#include "FFmpegAudioEncoder.h"
#include "FFmpegVideoDecoder.h"
#include "FFmpegVideoEncoder.h"
#include "FFmpegAudioResample.h"
#include "FFmpegVideoScale.h"
#include "FFmpegOSD.h"
#include <time_expend.cpp>

DOM_CLASS_EXPORT_BEG
av_register_all();
avcodec_register_all();
avfilter_register_all();
avformat_network_init();
DOM_CLASS_EXPORT(CMediaType,NULL,0,"media_type",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(CFFmpegDemuxer,FILTER_TYPE_NAME,FT_Source,"ffmpeg demuxer",MAKE_VERSION(0,0,0,0),(void*)CFFmpegDemuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegMuxer,FILTER_TYPE_NAME,FT_Render,"ffmpeg muxer",MAKE_VERSION(0,0,0,0),(void*)CFFmpegMuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegAudioDecoder,FILTER_TYPE_NAME,FT_Transform,"ffmpeg audio decoder",MAKE_VERSION(0,0,0,0),(void*)CFFmpegAudioDecoder::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegAudioEncoder,FILTER_TYPE_NAME,FT_Transform,"ffmpeg audio encoder",MAKE_VERSION(0,0,0,0),(void*)CFFmpegAudioEncoder::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegVideoDecoder,FILTER_TYPE_NAME,FT_Transform,"ffmpeg video decoder",MAKE_VERSION(0,0,0,0),(void*)CFFmpegVideoDecoder::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegVideoEncoder,FILTER_TYPE_NAME,FT_Transform,"ffmpeg video encoder",MAKE_VERSION(0,0,0,0),(void*)CFFmpegVideoEncoder::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegAudioResample,FILTER_TYPE_NAME,FT_Transform,"ffmpeg audio resample",MAKE_VERSION(0,0,0,0),(void*)CFFmpegAudioResample::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegVideoScale,FILTER_TYPE_NAME,FT_Transform,"ffmpeg video scale",MAKE_VERSION(0,0,0,0),(void*)CFFmpegVideoScale::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFFmpegOSD,FILTER_TYPE_NAME,FT_Transform,"ffmpeg OSD filter",MAKE_VERSION(0,0,0,0),(void*)CFFmpegOSD::FilterQuery,NULL)
DOM_CLASS_EXPORT_END

int gcd(int a,int b)
{
    while(a!=b)
    {
       if(a>b)
           a-=b;
       else
           b-=a;
    }
    return a;
}

//HRESULT WriteFrameBuffer(IMediaFrame* pFrame,void* pBuf,size_t szBuf)
//{
//    HRESULT hr;
//	JIF(pFrame->Alloc(szBuf));
//    JIF(pFrame->SetBuf((uint8_t*)pBuf,szBuf));
//	return hr;
//}

HRESULT AVPacketToFrame(IMediaFrame* pFrame, AVPacket& packet,const AVRational& base,int frame_size)
{
	JCHK(NULL != pFrame,E_INVALIDARG);
    HRESULT hr;
	pFrame->info.flag = packet.flags;
    pFrame->info.dts = AV_NOPTS_VALUE == packet.dts ? MEDIA_FRAME_NONE_TIMESTAMP : int64_t(FRAME_TIMEBASE.den * av_q2d(base) * packet.dts + 0.5);
    pFrame->info.pts = AV_NOPTS_VALUE == packet.pts ? MEDIA_FRAME_NONE_TIMESTAMP : int64_t(FRAME_TIMEBASE.den * av_q2d(base) * packet.pts + 0.5);
    pFrame->info.duration = 0 == packet.duration ? 0 : int64_t(packet.duration * FRAME_TIMEBASE.den * av_q2d(base) + 0.5);
    pFrame->info.samples = frame_size;
    JIF(pFrame->SetBuf(0,packet.size,packet.data));
    return hr;
}

HRESULT FrameToAVPacket(AVPacket* pPacket,IMediaFrame* pFrame,const AVRational* base)
{
	JCHK(NULL != pFrame,E_INVALIDARG);
	JCHK(NULL != pPacket,E_INVALIDARG);
	pPacket->flags = (AV_PKT_FLAG_KEY|AV_PKT_FLAG_CORRUPT) & pFrame->info.flag;
	pPacket->dts = NULL == base ? pFrame->info.dts : (MEDIA_FRAME_NONE_TIMESTAMP == pFrame->info.dts ? AV_NOPTS_VALUE : av_rescale_q(pFrame->info.dts , FRAME_TIMEBASE, *base));
	pPacket->pts = NULL == base ? pFrame->info.pts : (MEDIA_FRAME_NONE_TIMESTAMP == pFrame->info.pts ? AV_NOPTS_VALUE : av_rescale_q(pFrame->info.pts , FRAME_TIMEBASE, *base));
	pPacket->duration = NULL == base ? pFrame->info.duration : av_rescale_q(pFrame->info.duration , FRAME_TIMEBASE, *base);
	const IMediaFrame::buf* buf;
	JCHK(buf = pFrame->GetBuf(),E_INVALIDARG);
	pPacket->data = (uint8_t*)buf->data;
	pPacket->size = buf->size;
	return S_OK;
}

HRESULT AVFrameToFrame(IMediaType* pMT,IMediaFrame* pDest,AVFrame* pSour)
{
	pDest->info.flag |= 0 == pSour->key_frame ? 0 : MEDIA_FRAME_FLAG_SYNCPOINT;
	pDest->info.dts = pSour->pkt_dts;
	pDest->info.pts = pSour->pkt_pts;
    //pDest->info.samples = pSour->nb_samples;
    return pMT->ArrayToFrame(pDest,(const uint8_t**)pSour->data,(const int*)pSour->linesize);
}

HRESULT FrameToAVFrame(IMediaType* pMT,AVFrame* pDest,IMediaFrame* pSour,AVCodecContext* ctxCodec)
{
	pDest->key_frame = 0 == (MEDIA_FRAME_FLAG_SYNCPOINT&pSour->info.flag) ? 0 : 1;
    pDest->pkt_pts = MEDIA_FRAME_NONE_TIMESTAMP == pSour->info.pts ? AV_NOPTS_VALUE : av_rescale_q(pSour->info.pts , FRAME_TIMEBASE, ctxCodec->time_base);
    pDest->pkt_dts = MEDIA_FRAME_NONE_TIMESTAMP == pSour->info.dts ? AV_NOPTS_VALUE : av_rescale_q(pSour->info.dts , FRAME_TIMEBASE, ctxCodec->time_base);
	pDest->pts = pDest->pkt_pts;
	if(AVMEDIA_TYPE_VIDEO == ctxCodec->codec_type)
	{
        pDest->width = ctxCodec->width;
        pDest->height = ctxCodec->height;
        pDest->format = ctxCodec->pix_fmt;
        pDest->sample_aspect_ratio = ctxCodec->sample_aspect_ratio;
	}
	else if(AVMEDIA_TYPE_AUDIO == ctxCodec->codec_type)
	{
        pDest->format = ctxCodec->sample_fmt;
        pDest->nb_samples = ctxCodec->frame_size;
        pDest->channels = ctxCodec->channels;
        pDest->sample_rate = ctxCodec->sample_rate;
	}
    return pMT->FrameToArray(pDest->data,pDest->linesize,pSour);
}

void GetAudioSampleRate(const int* supported_samplerates,int& sample_rate)
{
    int i=0;
    int sample_rate_result = 0,delta = 0;
    do
    {
        int sample_rate_temp = supported_samplerates[i];
        if(0 == sample_rate_temp)
            break;
        int delta_temp = abs(sample_rate_temp - sample_rate);
        if(0 == delta_temp)
        {
            delta = delta_temp;
            sample_rate_result = sample_rate_temp;
            break;
        }
        else if(0 == i || delta_temp < delta)
        {
            delta = delta_temp;
            sample_rate_result = sample_rate_temp;
        }
    }while(0 < ++i);
    sample_rate = sample_rate_result;
}

bool GetVideoDuration(const AVRational* supported_framerates,int64_t& duration)
{
    int i=0;
    int64_t delta = 0,result = 0;
    do
    {
        AVRational frame_rate = supported_framerates[i];
        if(0 == frame_rate.num || 0 == frame_rate.den)
        {
            break;
        }
        int64_t result_temp = int64_t(10000000/av_q2d(frame_rate));
        int64_t delta_temp = abs(result_temp - duration);
        if(0 == delta_temp)
        {
            delta = delta_temp;
            result = result_temp;
            break;
        }
        else if(0 == i || delta_temp < delta)
        {
            delta = delta_temp;
            result = result_temp;
        }
    }while(0 < ++i);
    if(duration != result)
    {
        duration = result;
        return false;
    }
    else
        return true;
}

void GetOption(void* obj,IProfile* pProfile)
{
    IProfile::val* pVal;
    IProfile::Name name = NULL;
    int rl = 0;
    while(NULL != (name = pProfile->Next(name)))
    {
        if(NULL != (pVal = pProfile->Read(name)) && NULL != (pVal->value))
        {
            if(true == STR_CMP(pVal->type,typeid(int).name())||
                true == STR_CMP(pVal->type,typeid(int64_t).name()))
                rl = av_opt_set_int(obj,name,*(int*)pVal->value,0);
            else if(true == STR_CMP(pVal->type,typeid(float).name())||
                true == STR_CMP(pVal->type,typeid(double).name()))
                rl = av_opt_set_double(obj,name,*(float*)pVal->value,0);
            else if(true == STR_CMP(pVal->type,typeid(char*).name())||
                true == STR_CMP(pVal->type,typeid(const char*).name()))
                rl = av_opt_set(obj,name,(char*)pVal->value,0);
            else
            {
                LOG(0,"option:%s type:%s is not support",name,pVal->type);
            }
            if(AVERROR_OPTION_NOT_FOUND == rl)
            {
                LOG(0,"option:%s is not find",name);
            }
            else if(ERANGE == rl)
            {
                LOG(0,"option:%s value is out of range",name);
            }
            else if(EINVAL == rl)
            {
                LOG(0,"option:%s value is not valid",name);
            }
            else if(0 != rl)
            {
                LOG(0,"option:%s set fail",name);
            }
        }
    }
}

bool SetFilterInfo(IFilter* pFilter,int64_t* start_time,int64_t* length,int64_t* bitrate)
{
    dom_ptr<IProfile> spProfile;
    JCHK(spProfile.p = static_cast<IProfile*>(pFilter->Query(IID(IProfile))),false);
    if(NULL != start_time)
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP != *start_time)
        {
            JCHK(spProfile->Write(STREAM_START_TIME_KEY,start_time),false);
        }
        else
        {
            spProfile->Erase(STREAM_START_TIME_KEY);
        }
    }
    if(NULL != length)
    {
        if(0 < *length)
        {
            JCHK(spProfile->Write(STREAM_LENGTH_KEY,length),false);
        }
        else
        {
            spProfile->Erase(STREAM_LENGTH_KEY);
        }
    }
    if(NULL != bitrate)
    {
        if(0 < *bitrate)
        {
            JCHK(spProfile->Write(STREAM_BITRATE_KEY,bitrate),false);
        }
        else
        {
            spProfile->Erase(STREAM_BITRATE_KEY);
        }
    }
    return true;
}

void get_pps_sps(uint8_t *buf, uint32_t size, bool is264, uint32_t &offset, uint32_t &len)
{
    len = 0;
    struct NAL
    {
        uint32_t offfset;
        uint32_t len;
    };
    NAL _nals[1024];
    if (size < 8 || buf == NULL)
        return;
    uint32_t idx = 0;

    bool foundFirstStartCode = false;
    memset(_nals, 0, sizeof(_nals));
    for (uint32_t i = 4; i < size && idx < sizeof(_nals)/sizeof(NAL); ++i)
    {
        if (0x01 == buf[i-1] && 0x00 == buf[i-2] && 0x00 == buf[i-3])
        {
            if (false == foundFirstStartCode)
            {
                foundFirstStartCode = true;
                _nals[idx].offfset = i;
                idx++;
                continue;
            }
            _nals[idx].offfset = i;
            _nals[idx-1].len = i - _nals[idx-1].offfset - (buf[i-4] == 0 ?4:3);
            ++idx;
        }
    }
    if (idx > 0)
        _nals[idx-1].len = size - _nals[idx-1].offfset;

    static const uint8_t AVCPPS = 0x8;
    static const uint8_t AVCSPS = 0x7;
    static const uint8_t HEVCPPS = 34;
    static const uint8_t HEVCSPS = 33;
    static const uint8_t HEVCVPS = 32;
    for (int i = 1; i < 1024 && _nals[i].len != 0; ++i)
    {
        uint8_t pps, sps;
        uint8_t rsh = 0;
        pps = AVCPPS;
        sps = AVCSPS;
        uint8_t mark = 0x1f;
        if (false == is264)
        {
            rsh = 1;
            pps = HEVCPPS;
            sps = HEVCSPS;
            mark = 0x3f;
        }
        if(((buf[_nals[i].offfset] >> rsh) & mark) == pps // pps
                && ((buf[_nals[i-1].offfset] >> rsh) & mark) == sps) // sps
        {
            if (is264 == false)
            {
                 if (i >= 2
                         && ((buf[_nals[i-2].offfset] >> rsh & mark) == HEVCVPS))
                 {
                     uint32_t o = _nals[i].offfset+_nals[i].len;
                     len =  o - _nals[i-2].offfset;
                     offset = _nals[i-2].offfset;
                 }
            }
            else
            {
                uint32_t o = _nals[i].offfset+_nals[i].len;
                len =  o - _nals[i-1].offfset;
                offset = _nals[i-1].offfset;
            }
        }
    }
    if(0 < len)
    {
        if(3 <= offset && 0 == buf[offset-3])
        {
            offset -= 3;
            len += 3;
        }
        else
        {
            offset -= 2;
            len += 2;
        }
    }
}
int64_t GetColockCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec * 10000000 + ts.tv_nsec / 100;
}
