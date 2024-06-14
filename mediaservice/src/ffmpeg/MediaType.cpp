#include "MediaType.h"
static const AVCodecDescriptor pcm_descriptor =
{
    (AVCodecID)MST_PCM,
    AVMEDIA_TYPE_AUDIO,
    "pcm",
    "PCM audio",
    AV_CODEC_PROP_LOSSLESS
};

static const AVCodecDescriptor gosun_descriptor =
{
    (AVCodecID)MST_GOSUN_DATA,
    AVMEDIA_TYPE_DATA,
    "gosun",
    "gosun foramt stream",
    0
};

static const AVCodecDescriptor flv_tag_descriptor =
{
    (AVCodecID)MST_FLV_TAG,
    AVMEDIA_TYPE_DATA,
    "flv",
    "flv tag stream",
    0
};

static const AVCodecDescriptor flv_tag_body_descriptor =
{
    (AVCodecID)MST_FLV_TAG_BODY,
    AVMEDIA_TYPE_DATA,
    "flv_body",
    "flv tag body stream",
    0
};

static const AVCodecDescriptor ts_descriptor =
{
    (AVCodecID)MST_MPEG2TS,
    AVMEDIA_TYPE_DATA,
    "ts",
    "mpeg2 ts stream",
    0
};

static const AVCodecDescriptor hls_descriptor =
{
    (AVCodecID)MST_HLS,
    AVMEDIA_TYPE_DATA,
    "m3u8",
    "hls stream",
    0
};

CMediaType::CMediaType()
:m_major(MMT_UNKNOWN)
,m_pDesp(NULL)
,m_pix_fmt(VMT_NONE)
,m_width(0)
,m_height(0)
,m_ratioX(0)
,m_ratioY(0)
,m_duration(0)
,m_sample_fmt(AMT_NONE)
,m_channel_layout(0)
,m_channels(0)
,m_sample_rate(0)
,m_frame_size(0)
{
}

bool CMediaType::FinalConstruct(Interface* pOuter,void* pParam)
{
    return m_spProfile.Create(CLSID_CMemProfile,(IMediaType*)this,true);
}

bool CMediaType::FinalDestructor(bool finally)
{
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CMediaType)
DOM_QUERY_IMPLEMENT(IMediaType)
DOM_QUERY_IMPLEMENT(ISerialize)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CMediaType::SetMajor(MediaMajorType major)
{
    JCHK(MMT_UNKNOWN < major && MMT_NB > major,E_INVALIDARG);
    if(NULL != m_pDesp)
    {
        if(major != (MediaMajorType)m_pDesp->type)
            m_pDesp = NULL;
    }
    m_major = major;
    return S_OK;
}

STDMETHODIMP CMediaType::SetMajor(const char* pName)
{
    return SetMajor(GetMajorByName(pName));
}

STDMETHODIMP_(MediaMajorType) CMediaType::GetMajor()
{
    return m_major;
}

STDMETHODIMP CMediaType::SetSub(MediaSubType sub)
{
    if(MST_NONE == sub)
    {
        m_pDesp = NULL;
    }
    else
    {
        JCHK(NULL != (m_pDesp = GetDescriptor(sub)),E_INVALIDARG);
        m_major = (MediaMajorType)m_pDesp->type;
    }
    return S_OK;
}

STDMETHODIMP CMediaType::SetSub(const char* pName)
{
    const AVCodecDescriptor* pDesp;

    if(NULL != (pDesp = GetDescriptor(pName)))
    {
        m_pDesp = pDesp;
        m_major = (MediaMajorType)m_pDesp->type;
        return S_OK;
    }
    else
        return E_INVALIDARG;
}

STDMETHODIMP_(MediaSubType) CMediaType::GetSub()
{
    return NULL == m_pDesp ? MST_NONE : (MediaSubType)m_pDesp->id;
}

STDMETHODIMP_(uint32_t) CMediaType::GetProps()
{
    return NULL == m_pDesp ? 0 : (uint32_t)m_pDesp->props;
}

STDMETHODIMP_(const char*) CMediaType::GetMajorName()
{
    return av_get_media_type_string((AVMediaType)m_major);
}

STDMETHODIMP_(const char*) CMediaType::GetSubName()
{
    return NULL == m_pDesp ? NULL : m_pDesp->name;
}

STDMETHODIMP_(const char*) CMediaType::GetSubLongName()
{
    return NULL == m_pDesp ? NULL : m_pDesp->long_name;
}

STDMETHODIMP_(bool) CMediaType::IsCompress()
{
    if(NULL == m_pDesp)
        return false;
    if(AVMEDIA_TYPE_VIDEO == m_pDesp->type)
        return m_pDesp->id != AV_CODEC_ID_RAWVIDEO;
    else
        return m_pDesp->props != AV_CODEC_PROP_LOSSLESS;
}

STDMETHODIMP_(uint32_t) CMediaType::Compare(IMediaType* pMT)
{
    return Compare(this,pMT);
}

STDMETHODIMP CMediaType::CopyFrom(IMediaType* pMT,uint32_t flag)
{
    return Copy(this,pMT,flag);
}

STDMETHODIMP CMediaType::CopyTo(IMediaType* pMT,uint32_t flag)
{
    return Copy(pMT,this,flag);
}

STDMETHODIMP CMediaType::Clear()
{
    HRESULT hr;
    JIF(SetSub(MST_NONE));
    dom_ptr<IProfile> spProfile(this);
    spProfile->Clear();
    return hr;
}

STDMETHODIMP_(uint32_t) CMediaType::GetFourcc(uint32_t flag)
{
    uint32_t fourcc = 0;
    if(MST_RAWVIDEO == GetSub() && m_pix_fmt > VMT_NONE && m_pix_fmt < VMT_NB)
    {
        const VideoMediaTypeTag *tags = video_media_type_tags;
        while (VMT_NONE != tags->vmt)
        {
            if (tags->vmt == m_pix_fmt && (flag == (tags->flag&flag)))
            {
                fourcc = tags->fourcc;
                break;
            }
            tags++;
        }
    }
    return fourcc;
}

STDMETHODIMP CMediaType::SetFourcc(uint32_t fourcc)
{
    JCHK(0 != fourcc,VMT_NONE);
    if(MST_RAWVIDEO == GetSub())
    {
        const VideoMediaTypeTag *tags = video_media_type_tags;
        while (tags->fourcc != 0)
        {
            if (tags->fourcc == fourcc)
            {
                return SetVideoInfo((VideoMediaType*)&tags->vmt,NULL,NULL,NULL,NULL,NULL);
            }
            tags++;
        }
    }
    return S_OK;
}

STDMETHODIMP CMediaType::GetStreamInfo(int64_t* start_time,int64_t* length,int64_t* bitrate,bool* global_header,uint8_t** extra_data,int* extra_size)
{
    HRESULT hr = S_OK;
    dom_ptr<IProfile> spProfile(this);
    if(NULL != start_time)
    {
        IProfile::val* pVal = spProfile->Read(STREAM_START_TIME_KEY);
        if(NULL != pVal)
        {
            if(true == STR_CMP(pVal->type,typeid(int64_t).name()))
                *start_time = *(int64_t*)pVal->value;
            else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
                true == STR_CMP(pVal->type,typeid(char*).name()))
            {
                string val = (const char*)pVal->value;
                transform(val.begin(),val.end(),val.begin(),::tolower);
                if(val == STREAM_START_TIME_CLOCK_VALUE)
                    *start_time = GetColockCount();
                else
                    hr = E_INVALIDARG;
            }
            else
                hr = E_INVALIDARG;
        }
        else
            hr = E_INVALIDARG;
    }
    if(NULL != length)
    {
        if(false == spProfile->Read(STREAM_LENGTH_KEY,length))
            hr = E_INVALIDARG;
    }
    if(NULL != bitrate)
    {
        IProfile::val* pVal = spProfile->Read(STREAM_BITRATE_KEY);
        if(NULL != pVal)
        {
            if(true == STR_CMP(pVal->type,typeid(int64_t).name()))
            {
                *bitrate = *(int64_t*)pVal->value;
            }
            else if(true == STR_CMP(pVal->type,typeid(int).name()))
            {
                *bitrate = *(int*)pVal->value;
            }
            else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
                true == STR_CMP(pVal->type,typeid(char*).name()))
            {
                char* pUnit = NULL;
                double val = strtod((const char*)pVal->value, &pUnit);
                if(0.0 < val)
                {
                    if(*pUnit == 'k')
                        *bitrate = int64_t(val * 1000.0);
                    else if(*pUnit == 'K')
                        *bitrate = int64_t(val * 8000.0);
                    else if(*pUnit == 'm')
                        *bitrate = int64_t(val * 1000000.0);
                    else if(*pUnit == 'M')
                        *bitrate = int64_t(val * 8000000.0);
                    else
                        *bitrate = (int64_t)val;
                }
            }
            else
                hr = E_INVALIDARG;
        }
        else
            hr = E_INVALIDARG;
    }
    if(NULL != global_header)
    {
        if(false == spProfile->Read(STREAM_GLOBALHEADER_KEY,global_header))
            *global_header = false;
    }
    if(NULL != extra_data || NULL != extra_size)
    {
        IProfile::val* pVal = spProfile->Read(STREAM_EXTRADATA_KEY);

        if(NULL != pVal)
        {
            if(NULL != extra_data)
                *extra_data = (uint8_t*)pVal->value;
            if(NULL != extra_size)
                *extra_size = pVal->len;
        }
        else
        {
            if(NULL != extra_data)
                *extra_data = NULL;
            if(NULL != extra_size)
                *extra_size = 0;
        }
    }
    return hr;
}

STDMETHODIMP CMediaType::SetStreamInfo(int64_t* start_time,int64_t* length,int64_t* bitrate,bool* global_header,uint8_t* extra_data,int* extra_size,uint8_t** extra_data_out)
{
    dom_ptr<IProfile> spProfile(this);
    if(NULL != start_time)
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP != *start_time)
        {
            JCHK(spProfile->Write(STREAM_START_TIME_KEY,start_time),E_FAIL);
        }
    }

    if(NULL != length)
    {
        if(0 < *length)
        {
            JCHK(spProfile->Write(STREAM_LENGTH_KEY,length),E_FAIL);
        }
    }

    if(NULL != bitrate)
    {
        if(0 < *bitrate)
        {
            JCHK(spProfile->Write(STREAM_BITRATE_KEY,bitrate),E_FAIL);
        }
    }

    if(NULL != global_header)
    {
        JCHK(spProfile->Write(STREAM_GLOBALHEADER_KEY,global_header),E_FAIL);
    }
    else
    {
        spProfile->Erase(STREAM_GLOBALHEADER_KEY);
    }
    if(NULL != extra_size && (NULL != extra_data || NULL != extra_data_out))
    {
        IProfile::val* pVal;
        JCHK(0 < *extra_size,E_INVALIDARG);
        JCHK(NULL != (pVal = spProfile->Write(STREAM_EXTRADATA_KEY,IID(uint8_t*),extra_data,(size_t)*extra_size)),E_FAIL);
        if(NULL != extra_data_out)
        {
            JCHK(NULL != (*extra_data_out = (uint8_t*)pVal->value),E_FAIL);
        }
    }
    else
    {
        spProfile->Erase(STREAM_EXTRADATA_KEY);
    }
    return S_OK;
}

STDMETHODIMP CMediaType::GetVideoInfo(VideoMediaType* pix_fmt,int* width,int* height,int* ratioX,int* ratioY,int64_t* duration)
{
    HRESULT hr;

    dom_ptr<IProfile> spProfile(this);

    if(NULL != pix_fmt)
    {
        IProfile::val* pVal = spProfile->Read(VIDEO_MEDIA_TYPE_KEY);
        if(NULL != pVal)
        {
            if(true == STR_CMP(pVal->type,typeid(VideoMediaType).name()))
                *pix_fmt = *(VideoMediaType*)pVal->value;
            else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
                true == STR_CMP(pVal->type,typeid(char*).name()))
                *pix_fmt = (VideoMediaType)av_get_pix_fmt((const char*)pVal->value);
        }
    }
    if(NULL != width)
        spProfile->Read(VIDEO_WIDTH_KEY,width);
    if(NULL != height)
        spProfile->Read(VIDEO_HEIGHT_KEY,height);
    if(NULL != ratioX)
        spProfile->Read(VIDEO_RATIOX_KEY,ratioX);
    if(NULL != ratioY)
        spProfile->Read(VIDEO_RATIOY_KEY,ratioY);
    if(NULL != duration)
    {
        IProfile::val* pVal = spProfile->Read(VIDEO_FPS_KEY);
        if(NULL != pVal)
        {
            if(true == STR_CMP(pVal->type,typeid(int).name()))
            {
                int val = *(int*)pVal->value;
                if(val > 0)
                    *duration = int64_t(10000000.0 / val);
            }
            else if(true == STR_CMP(pVal->type,typeid(double).name()))
            {
                double val = *(double*)pVal->value;
                if(val > 0.0)
                    *duration = int64_t(10000000.0 / val);
            }
        }
    }
    JIF(InternalSetVideoInfo(pix_fmt,width,height,ratioX,ratioY,duration));
    if(NULL != ratioX || NULL != ratioY)
    {
        int ratioX_tmp = 0 == m_ratioX ? m_width : m_ratioX;
        int ratioY_tmp = 0 == m_ratioY ? m_height : m_ratioY;
        int t = gcd(ratioX_tmp,ratioY_tmp);
        if(1 < t)
        {
            ratioX_tmp /= t;
            ratioY_tmp /= t;
            if(NULL != ratioX)
                *ratioX = ratioX_tmp;
            if(NULL != ratioY)
                *ratioY = ratioY_tmp;
        }
    }
    return hr;
}

STDMETHODIMP CMediaType::SetVideoInfo(VideoMediaType* pix_fmt,int* width,int* height,int* ratioX,int* ratioY,int64_t* duration)
{
    HRESULT hr;
    JIF(InternalSetVideoInfo(pix_fmt,width,height,ratioX,ratioY,duration));
    dom_ptr<IProfile> spProfile(this);
    if(NULL != pix_fmt)
    {
        JCHK(spProfile->Write(VIDEO_MEDIA_TYPE_KEY,pix_fmt),E_FAIL);
    }
    if(NULL != width)
    {
        JCHK(spProfile->Write(VIDEO_WIDTH_KEY,width),E_FAIL);
    }
    if(NULL != height)
    {
        JCHK(spProfile->Write(VIDEO_HEIGHT_KEY,height),E_FAIL);
    }
    if(NULL != ratioX)
    {
        JCHK(spProfile->Write(VIDEO_RATIOX_KEY,ratioX),E_FAIL);
    }
    if(NULL != ratioY)
    {
        JCHK(spProfile->Write(VIDEO_RATIOY_KEY,ratioY),E_FAIL);
    }
    if(NULL != duration)
    {
        JCHK(0 < *duration,E_INVALIDARG);
        double fps = 10000000.0 / (*duration);
        JCHK(spProfile->Write(VIDEO_FPS_KEY,fps),E_FAIL);
    }
    return hr;
}

STDMETHODIMP CMediaType::GetAudioInfo(AudioMediaType* sample_fmt,uint64_t* channel_layout,int* channels,int* sample_rate,int* frame_size)
{
    dom_ptr<IProfile> spProfile(this);
    if(NULL != sample_fmt)
    {
        IProfile::val* pVal = spProfile->Read(AUDIO_MEDIA_TYPE_KEY);
        if(NULL != pVal)
        {
            if(true == STR_CMP(pVal->type,typeid(AudioMediaType).name()))
                *sample_fmt = *(AudioMediaType*)pVal->value;
            else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
                true == STR_CMP(pVal->type,typeid(char*).name()))
                *sample_fmt = (AudioMediaType)av_get_sample_fmt((const char*)pVal->value);
        }
    }
    if(NULL != channel_layout)
    {
        IProfile::val* pVal = spProfile->Read(AUDIO_CHANNEL_LAYOUT_KEY);
        if(NULL != pVal)
        {
            if(true == STR_CMP(pVal->type,typeid(uint64_t).name()))
                *channel_layout = *(uint64_t*)pVal->value;
            else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
                true == STR_CMP(pVal->type,typeid(char*).name()))
                *channel_layout = av_get_channel_layout((const char*)pVal->value);
        }
    }
    if(NULL != channels)
        spProfile->Read(AUDIO_CHANNELS_KEY,channels);
    if(NULL != sample_rate)
        spProfile->Read(AUDIO_SAMPLE_RATE_KEY,sample_rate);
    if(NULL != frame_size)
        spProfile->Read(AUDIO_FRAME_SIZE_KEY,frame_size);
    return InternalSetAudioInfo(sample_fmt,channel_layout,channels,sample_rate,frame_size);
}

STDMETHODIMP CMediaType::SetAudioInfo(AudioMediaType* sample_fmt,uint64_t* channel_layout,int* channels,int* sample_rate,int* frame_size)
{
    HRESULT hr = S_OK;
    JIF(InternalSetAudioInfo(sample_fmt,channel_layout,channels,sample_rate,frame_size));
    dom_ptr<IProfile> spProfile(this);
    if(NULL != sample_fmt)
    {
        JCHK(spProfile->Write(AUDIO_MEDIA_TYPE_KEY,sample_fmt),E_FAIL);
    }
    if(NULL != channel_layout)
    {
        JCHK(spProfile->Write(AUDIO_CHANNEL_LAYOUT_KEY,channel_layout),E_FAIL);
        JCHK(spProfile->Write(AUDIO_CHANNELS_KEY,m_channels),E_FAIL);
    }
    else if(NULL != channels)
    {
        JCHK(spProfile->Write(AUDIO_CHANNELS_KEY,channels),E_FAIL);
        JCHK(spProfile->Write(AUDIO_CHANNEL_LAYOUT_KEY,m_channel_layout),E_FAIL);
    }
    if(NULL != sample_rate)
    {
        JCHK(spProfile->Write(AUDIO_SAMPLE_RATE_KEY,sample_rate),E_FAIL);
    }
    if(NULL != frame_size)
    {
        JCHK(spProfile->Write(AUDIO_FRAME_SIZE_KEY,frame_size),E_FAIL);
    }
    return hr;
}

STDMETHODIMP CMediaType::FrameAlloc(IMediaFrame* frm)
{
    JCHK(NULL != frm,E_INVALIDARG);

    MediaSubType sub = GetSub();
    JCHK1(MST_RAWVIDEO == sub || MST_PCM == sub,E_INVALIDARG,"%s can not alloc frame",GetSubName());

    HRESULT hr = S_OK;

    if(MST_RAWVIDEO == sub)
    {
        int szBuf;
        JCHK(VMT_NONE < m_pix_fmt && VMT_NB > m_pix_fmt && 0 < m_width && 0 < m_height,E_FAIL);
        int stride = FFALIGN(m_width,VIDEO_ALIGN*2);
        JCHK(0 <(szBuf = av_image_get_buffer_size((AVPixelFormat)m_pix_fmt,stride,m_height,1)),E_FAIL);
        JIF(frm->SetBuf(0,(uint32_t)szBuf));
        frm->info.stride = stride;
        frm->info.duration = m_duration;
    }
    else if(MST_PCM == sub)
    {
        int szBuf;
        JCHK(AMT_NONE < m_sample_fmt && AMT_NB > m_sample_fmt && 0 < m_channels && 0 < m_frame_size,E_FAIL);
		JCHK(0< (szBuf = av_samples_get_buffer_size(NULL,m_channels,m_frame_size,(AVSampleFormat)m_sample_fmt,AUDIO_ALIGN)),E_FAIL);
        JIF(frm->SetBuf(0,(uint32_t)szBuf));
        frm->info.samples = m_frame_size;
        frm->info.duration = m_duration;
    }
    return hr;
}

STDMETHODIMP CMediaType::FrameToArray(uint8_t** dst_data,int* dst_linesize,IMediaFrame* src)
{
    JCHK(NULL != dst_data,E_INVALIDARG);
    JCHK(NULL != dst_linesize,E_INVALIDARG);
    JCHK(NULL != src,E_INVALIDARG);

    MediaSubType sub = GetSub();
    JCHK1(MST_RAWVIDEO == sub || MST_PCM == sub,E_INVALIDARG,"%s can not fill array",GetSubName());

    HRESULT hr = S_OK;
    const IMediaFrame::buf* buf;
    JCHK(buf = src->GetBuf(),E_INVALIDARG);

    if(MST_RAWVIDEO == sub)
    {
        JCHK(m_width <= src->info.stride,E_INVALIDARG);
        JCHK(VMT_NONE < m_pix_fmt && VMT_NB > m_pix_fmt && 0 < m_width && 0 < m_height,E_FAIL);
        JCHK(0 < av_image_fill_arrays(dst_data,dst_linesize,(uint8_t*)buf->data,(AVPixelFormat)m_pix_fmt,src->info.stride,m_height,1),E_FAIL);
    }
    else if(MST_PCM == sub)
    {
        JCHK(0 < src->info.samples,E_INVALIDARG);
        JCHK(AMT_NONE < m_sample_fmt && AMT_NB > m_sample_fmt && 0 < m_channels && 0 < m_frame_size,E_FAIL);
		JCHK(0 < av_samples_fill_arrays(dst_data,dst_linesize,(uint8_t*)buf->data,m_channels,src->info.samples,(AVSampleFormat)m_sample_fmt,AUDIO_ALIGN),E_FAIL);
    }
    else
    {
        JCHK(false,E_FAIL);
    }
    return hr;
}

STDMETHODIMP CMediaType::ArrayToFrame(IMediaFrame* dst,const uint8_t** src_data,const int* src_linesize)
{
    JCHK(NULL != dst,E_INVALIDARG);
    JCHK(NULL != src_data,E_INVALIDARG);
    JCHK(NULL != src_linesize,E_INVALIDARG);
    HRESULT hr = S_OK;
    MediaSubType sub = GetSub();
    JCHK1(MST_RAWVIDEO == sub || MST_PCM == sub,E_INVALIDARG,"%s can not fill frame",GetSubName());
    JIF(FrameAlloc(dst));

    if(MST_RAWVIDEO == GetSub())
    {
        uint8_t *dst_data[4]; int dst_linesize[4];
        JIF(FrameToArray(dst_data,dst_linesize,dst));
        av_image_copy(dst_data,dst_linesize,src_data,src_linesize,(AVPixelFormat)m_pix_fmt,m_width,m_height);
    }
    else if(MST_PCM == GetSub())
    {
        uint8_t *dst_data[4]; int dst_linesize[4];
        JIF(FrameToArray(dst_data,dst_linesize,dst));
        av_samples_copy(dst_data,(uint8_t*const*)src_data,0,0,dst->info.samples,m_channels,(AVSampleFormat)m_sample_fmt);
    }
    return hr;
}

STDMETHODIMP CMediaType::SetFrame(IMediaFrame* pFrame)
{
    m_spFrame = pFrame;
    return S_OK;
}

STDMETHODIMP_(IMediaFrame*) CMediaType::GetFrame()
{
    return m_spFrame.p;
}

STDMETHODIMP CMediaType::Load(IStream* pStream,uint8_t flag,void* param)
{
    HRESULT hr;
    JCHK(NULL != pStream,E_INVALIDARG);
    MediaMajorType maj;
    MediaSubType sub;
    JIF(pStream->Read(&maj,sizeof(maj),flag));
    JIF(SetMajor(maj));
    JIF(pStream->Read(&sub,sizeof(sub),flag));
    JIF(SetSub(sub));
    JIF(m_spProfile->Load(pStream,flag));
    return hr;
}

STDMETHODIMP CMediaType::Save(IStream* pStream,uint8_t flag,void* param)
{
    HRESULT hr;
    JCHK(NULL != pStream,E_INVALIDARG);
    MediaSubType sub = GetSub();
    JIF(pStream->Write(&m_major,sizeof(m_major),flag));
    JIF(pStream->Write(&sub,sizeof(sub),flag & (~IStream::WRITE_FLAG_REFFER)));
    JIF(m_spProfile->Save(pStream,flag));
    return hr;
}

HRESULT CMediaType::InternalSetVideoInfo(VideoMediaType* pix_fmt,int* width,int* height,int* ratioX,int* ratioY,int64_t* duration)
{
    HRESULT hr = S_OK;
    if(NULL != pix_fmt)
    {
        if(VMT_NONE < *pix_fmt && VMT_NB > *pix_fmt)
            m_pix_fmt = *pix_fmt;
        else
            hr = E_INVALIDARG;
    }
    if(NULL != width)
    {
        if(0 < *width)
            m_width = *width;
        else
            hr = E_INVALIDARG;
    }
    if(NULL != height)
    {
        if(0 < *height)
            m_height = *height;
        else
            hr = E_INVALIDARG;
    }
    if(NULL != ratioX)
    {
        if(0 <= *ratioX)
            m_ratioX = *ratioX;
        else
            hr = E_INVALIDARG;
    }
    if(NULL != ratioY)
    {
        if(0 <= *ratioY)
            m_ratioY = *ratioY;
        else
            hr = E_INVALIDARG;
    }
    if(NULL != duration)
    {
        if(0 < *duration)
            m_duration= *duration;
        else
            hr = E_INVALIDARG;
    }

    int t = gcd(m_ratioX,m_ratioY);
    if(1 < t)
    {
        m_ratioX /= t;
        m_ratioY /= t;
        if(NULL != ratioX)
            *ratioX = m_ratioX;
        if(NULL != ratioY)
            *ratioY = m_ratioY;
    }
    return hr;
}

HRESULT CMediaType::InternalSetAudioInfo(AudioMediaType* sample_fmt,uint64_t* channel_layout,int* channels,int* sample_rate,int* frame_size)
{
    HRESULT hr = S_OK;
    if(NULL != sample_fmt)
    {
        if(AMT_NONE < *sample_fmt && AMT_NB > *sample_fmt)
            m_sample_fmt = *sample_fmt;
        else
            hr = E_INVALIDARG;
    }
    if(NULL != channel_layout)
    {
        int channels_tmp;
        if(0 < *channel_layout && 0 < (channels_tmp = av_get_channel_layout_nb_channels(*channel_layout)))
        {
            m_channel_layout = *channel_layout;
            m_channels = channels_tmp;
            if(NULL != channels)
                *channels = channels_tmp;
        }
        else
            hr = E_INVALIDARG;
    }
    else if(NULL != channels)
    {
        int channel_layout_tmp;
        if(0 < *channels && 0 < (channel_layout_tmp = av_get_default_channel_layout(*channels)))
        {
            m_channels = *channels;
            m_channel_layout = channel_layout_tmp;
        }
        else
            hr = E_INVALIDARG;
    }
    if(NULL != sample_rate)
    {
        if(0 < *sample_rate)
            m_sample_rate = *sample_rate;
        else
            hr = E_INVALIDARG;
    }
    if(NULL != frame_size)
    {
        if(0 < *frame_size)
            m_frame_size = *frame_size;
        else
            hr = E_INVALIDARG;
    }
    if(0 < m_frame_size && 0 < m_sample_rate)
    {
        m_duration = av_rescale_rnd(m_frame_size, FRAME_TIMEBASE.den, m_sample_rate, AV_ROUND_NEAR_INF);
    }
    return hr;
}


const AVCodecDescriptor* CMediaType::GetDescriptor(MediaSubType type)
{
    if(MST_PCM == type)
        return &pcm_descriptor;
    else if(MST_GOSUN_DATA == type)
        return &gosun_descriptor;
    else if(MST_FLV_TAG == type)
        return &flv_tag_descriptor;
    else if(MST_FLV_TAG_BODY == type)
        return &flv_tag_body_descriptor;
    else if(MST_MPEG2TS == type)
        return &ts_descriptor;
    else if(MST_HLS == type)
        return &hls_descriptor;
    else
        return avcodec_descriptor_get((AVCodecID)type);
}

const AVCodecDescriptor* CMediaType::GetDescriptor(const char* pName)
{
    if(0 == strcmp(pName,pcm_descriptor.name))
        return &pcm_descriptor;
    else if(0 == strcmp(pName,gosun_descriptor.name))
        return &gosun_descriptor;
    else if(0 == strcmp(pName,flv_tag_descriptor.name))
        return &flv_tag_descriptor;
    else if(0 == strcmp(pName,flv_tag_body_descriptor.name))
        return &flv_tag_body_descriptor;
    else if(0 == strcmp(pName,ts_descriptor.name))
        return &ts_descriptor;
    else if(0 == strcmp(pName,hls_descriptor.name))
        return &hls_descriptor;
    else
        return avcodec_descriptor_get_by_name(pName);
}

MediaMajorType CMediaType::GetMajorByName(const char* pName)
{
    JCHK(NULL != pName,MMT_UNKNOWN);
    for(int i = AVMEDIA_TYPE_UNKNOWN+1 ; i < AVMEDIA_TYPE_NB ; ++i)
    {
        if(0 == strcmp(pName,av_get_media_type_string((AVMediaType)i)))
            return (MediaMajorType)i;
    }
    return MMT_UNKNOWN;
}

HRESULT CMediaType::Copy(IMediaType* pDest,IMediaType* pSour,uint32_t flag)
{
    HRESULT hr;
    JCHK(NULL != pDest,E_INVALIDARG);
    JCHK(NULL != pSour,E_INVALIDARG);
    if(pDest == pSour)
        return S_OK;

    dom_ptr<IProfile> spDest,spSour;
    JCHK(spDest.p = static_cast<IProfile*>(pDest->Query(IID(IProfile))),E_FAIL);
    JCHK(spSour.p = static_cast<IProfile*>(pSour->Query(IID(IProfile))),E_FAIL);
    MediaSubType dest = pDest->GetSub();
    MediaSubType sour = pSour->GetSub();
    if(dest != sour)
    {
        if(0 == flag || (MST_NONE == dest && 0 != (flag & COPY_FLAG_COMPILATIONS)) ||
            (MST_NONE != dest && MST_NONE != sour && 0 != (flag & COPY_FLAG_INTERSECTION)))
        {
            JIF(pDest->SetSub(sour));
        }
    }
    else
    {
        MediaMajorType dest = pDest->GetMajor();
        MediaMajorType sour = pSour->GetMajor();
        if(0 == flag || (MMT_UNKNOWN == dest && 0 != (flag & COPY_FLAG_COMPILATIONS)) ||
            (MMT_UNKNOWN != dest && MMT_UNKNOWN != sour && 0 != (flag & COPY_FLAG_INTERSECTION)))
        {
            JIF(pDest->SetMajor(sour));
        }
    }

    IMediaFrame* pDestFrame = pDest->GetFrame();
    IMediaFrame* pSourFrame = pSour->GetFrame();
    if(pDestFrame != pSourFrame)
    {
        if(0 == flag || (NULL == pDestFrame && 0 != (flag & COPY_FLAG_COMPILATIONS)) ||
           (NULL != pDestFrame && NULL != pSourFrame && 0 != (flag & COPY_FLAG_INTERSECTION)))
        {
            JIF(pDest->SetFrame(pSourFrame));
        }
    }
    JIF(spDest->CopyFrom(spSour,flag));
    return hr;
}

uint32_t CMediaType::Compare(IMediaType* pDest,IMediaType* pSour)
{
    JCHK(NULL != pDest,E_INVALIDARG);
    JCHK(NULL != pSour,E_INVALIDARG);

    if(pDest == pSour)
        return COMPARE_SAME_VALUE;
    MediaMajorType mmt;
    if(pDest->GetMajor() == (mmt = pSour->GetMajor()))
    {
        if(MST_NONE == pDest->GetSub())
            return COMPARE_SAME_VALUE;
        else if(MST_NONE == pSour->GetSub())
            return COMPARE_DIFFERENT_VALUE;
        else if(pDest->GetSub() == pSour->GetSub())
        {
            dom_ptr<IProfile> spDest,spSour;
            JCHK(spDest.p = static_cast<IProfile*>(pDest->Query(IID(IProfile))),E_FAIL);
            JCHK(spSour.p = static_cast<IProfile*>(pSour->Query(IID(IProfile))),E_FAIL);
            if(COMPARE_SAME_VALUE == spDest->Compare(spSour))
                return COMPARE_SAME_VALUE;
        }
        else if(MMT_DATA == mmt)
            return COMPARE_DIFFERENT_VALUE;

        if(false == pDest->IsCompress() && false == pSour->IsCompress())
        {
            return 1;
        }
        else if(pDest->IsCompress() != pSour->IsCompress())
        {
            return 2;
        }
        else if(true == pDest->IsCompress() && true == pSour->IsCompress())
        {
            return 3;
        }
    }
    return COMPARE_DIFFERENT_VALUE;
}

