#ifndef MEDIATYPE_H
#define MEDIATYPE_H

#include "stdafx.h"
#include <iProfile.h>

class CMediaType : public IMediaType
{
public:
    DOM_DECLARE(CMediaType)
    STDMETHODIMP SetMajor(MediaMajorType major);
	STDMETHODIMP SetMajor(const char* pName);
    STDMETHODIMP_(MediaMajorType) GetMajor();
	STDMETHODIMP SetSub(MediaSubType type);
	STDMETHODIMP SetSub(const char* pName);
	STDMETHODIMP_(MediaSubType) GetSub();
    STDMETHODIMP_(uint32_t) GetProps();
    STDMETHODIMP_(const char*) GetMajorName();
    STDMETHODIMP_(const char*) GetSubName();
    STDMETHODIMP_(const char*) GetSubLongName();
	STDMETHODIMP_(bool) IsCompress();
    STDMETHODIMP Compare(IMediaType* pMT);
    STDMETHODIMP CopyFrom(IMediaType* pMT,uint32_t flag);
    STDMETHODIMP CopyTo(IMediaType* pMT,uint32_t flag);
    STDMETHODIMP Clear();
    STDMETHODIMP_(uint32_t) GetFourcc(uint32_t flag);
    STDMETHODIMP SetFourcc(uint32_t fourcc);
    STDMETHODIMP GetStreamInfo(int64_t* start_time,int64_t* length,int64_t* bitrate,bool* global_header,uint8_t** extra_data,int* extra_size);
    STDMETHODIMP SetStreamInfo(int64_t* start_time,int64_t* length,int64_t* bitrate,bool* global_header,uint8_t* extra_data,int* extra_size);
    STDMETHODIMP GetVideoInfo(VideoMediaType* pix_fmt,int* width,int* height,int* ratioX,int* ratioY,int64_t* duration);
    STDMETHODIMP SetVideoInfo(VideoMediaType* pix_fmt,int* width,int* height,int* ratioX,int* ratioY,int64_t* duration);
    STDMETHODIMP GetAudioInfo(AudioMediaType* sample_fmt,uint64_t* channel_layout,int* channels,int* sample_rate,int* frame_size);
    STDMETHODIMP SetAudioInfo(AudioMediaType* sample_fmt,uint64_t* channel_layout,int* channels,int* sample_rate,int* frame_size);
    STDMETHODIMP FrameAlloc(IMediaFrame* frm);
    STDMETHODIMP FrameToArray(uint8_t** dst_data,int* dst_linesize,IMediaFrame* src);
    STDMETHODIMP ArrayToFrame(IMediaFrame* dst,const uint8_t** src_data,const int* src_linesize);
    //CMediaType
    HRESULT InternalSetVideoInfo(VideoMediaType* pix_fmt,int* width,int* height,int* ratioX,int* ratioY,int64_t* duration);
    HRESULT InternalSetAudioInfo(AudioMediaType* sample_fmt,uint64_t* channel_layout,int* channels,int* sample_rate,int* frame_size);
    static const AVCodecDescriptor* GetDescriptor(MediaSubType type);
    static const AVCodecDescriptor* GetDescriptor(const char* pName);
    static MediaMajorType GetMajorByName(const char* pName);
    static HRESULT Copy(IMediaType* pDest,IMediaType* pSour,uint32_t flag);
    static HRESULT Compare(IMediaType* pDest,IMediaType* pSour);
protected:
    MediaMajorType m_major;
    const AVCodecDescriptor* m_pDesp;
    dom_ptr<Interface> m_spProfile;

    VideoMediaType m_pix_fmt;
    int m_width;
    int m_height;
    int m_ratioX;
    int m_ratioY;
    int64_t m_duration;

    AudioMediaType m_sample_fmt;
    uint64_t m_channel_layout;
    int m_channels;
    int m_sample_rate;
    int m_frame_size;
};

#endif // MEDIATYPE_H
