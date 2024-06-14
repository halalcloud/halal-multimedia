#include "FFmpegMuxer.h"
#include "../src/Url.cpp"

CFFmpegMuxer::CStream::CStream(CFFmpegMuxer* pMuxer)
:m_pMuxer(pMuxer)
,m_pStream(NULL)
,m_pMT(NULL)
,m_ctxBSF(NULL)
,m_isGlobalHeader(false)
,m_preprocess(false)
,m_isEnd(false)
{

}
CFFmpegMuxer::CStream::~CStream()
{
    while(true == Pop()){}
}

HRESULT CFFmpegMuxer::CStream::Open()
{
    HRESULT hr = S_OK;

    if(m_pMT == NULL)
        return hr;

    if(NULL == m_pStream)
    {
        AVCodecID id = (AVCodecID)m_pMT->GetSub();
        JCHK(AV_CODEC_ID_NONE != id,E_INVALIDARG);
        JCHK(m_pStream = avformat_new_stream(m_pMuxer->m_ctxFormat,NULL),E_FAIL);
        m_pStream->codec->codec_type = (AVMediaType)m_pMT->GetMajor();
        m_pStream->codec->codec_id = id;
        m_pStream->id = m_pMuxer->m_ctxFormat->nb_streams;


        if(AVMEDIA_TYPE_VIDEO == m_pStream->codec->codec_type)
        {
            int64_t duration;
            int ratioX = 0,ratioY = 0;
            JIF(m_pMT->GetVideoInfo((VideoMediaType*)&m_pStream->codec->pix_fmt,
                &m_pStream->codec->width,&m_pStream->codec->height,
                &ratioX,&ratioY,&duration));

            m_pMuxer->m_ctxFormat->oformat->video_codec = m_pStream->codec->codec_id;

            if(0 == ratioX)
                ratioX = m_pStream->codec->width;
            if(0 == ratioY)
                ratioY = m_pStream->codec->height;

            unsigned int g = gcd(ratioX,ratioY);
            ratioX /= g;
            ratioY /= g;

            unsigned int sar_x = ratioX * m_pStream->codec->height;
            unsigned int sar_y = ratioY * m_pStream->codec->width;

            g = gcd(sar_x,sar_y);
            m_pStream->codec->sample_aspect_ratio.num = sar_x/g;
            m_pStream->codec->sample_aspect_ratio.den = sar_y/g;

            m_pStream->sample_aspect_ratio = m_pStream->codec->sample_aspect_ratio;

            m_pStream->time_base = VIDEO_TIMEBASE;
            m_pStream->codec->time_base = m_pStream->time_base;

            m_pStream->r_frame_rate.num = FRAME_TIMEBASE.den;
            m_pStream->r_frame_rate.den = duration;
            m_pStream->codec->framerate = m_pStream->r_frame_rate;
        }
        else if(AVMEDIA_TYPE_AUDIO == m_pStream->codec->codec_type)
        {
            JIF(m_pMT->GetAudioInfo((AudioMediaType*)&m_pStream->codec->sample_fmt,
                &m_pStream->codec->channel_layout,&m_pStream->codec->channels,
                &m_pStream->codec->sample_rate,&m_pStream->codec->frame_size));

            m_pMuxer->m_ctxFormat->oformat->audio_codec = m_pStream->codec->codec_id;

            m_pStream->time_base.num = 1;
            m_pStream->time_base.den = m_pStream->codec->sample_rate;
            m_pStream->codec->time_base = m_pStream->time_base;
        }
        else
            return E_INVALIDARG;

        double min = double(m_pMuxer->m_base.num) / m_pMuxer->m_base.den;
        double cur = double(m_pStream->time_base.num) / m_pStream->time_base.den;
        if(0 == min || cur < min)
            m_pMuxer->m_base = m_pStream->time_base;

        m_isGlobalHeader = m_pMuxer->m_isGlobalHeader;
        int extra_size = 0;
        uint8_t* extra_data = NULL;
        m_pMT->GetStreamInfo(NULL,NULL,&m_pStream->codec->bit_rate,&m_isGlobalHeader,&extra_data,&extra_size);

        if(NULL != extra_data && 0 < extra_size)
        {
            JCHK(NULL != (m_pStream->codec->extradata = (uint8_t*)av_realloc(m_pStream->codec->extradata,extra_size)),E_OUTOFMEMORY);
            memcpy(m_pStream->codec->extradata,extra_data,extra_size);
            m_pStream->codec->extradata_size = extra_size;
        }
        else
        {
            if(NULL != m_pStream->codec->extradata)
            {
                av_free(m_pStream->codec->extradata);
                m_pStream->codec->extradata = NULL;
            }
            m_pStream->codec->extradata_size = 0;
        }

        if(true == m_isGlobalHeader)
        {
            if(NULL == m_pStream->codec->extradata || 0 == m_pStream->codec->extradata_size)
                m_isGlobalHeader = false;
        }

        if(AV_CODEC_ID_H264 == m_pStream->codec->codec_id || AV_CODEC_ID_HEVC == m_pStream->codec->codec_id)
        {
            if(false == m_isGlobalHeader)
            {
                if(NULL == m_pStream->codec->extradata || 0 == m_pStream->codec->extradata_size)
                {
                    dom_ptr<IMediaFrame> spFrame;
                    JIF(Peek(&spFrame));
                    const IMediaFrame::buf* buf;
                    JCHK(buf = spFrame->GetBuf(),E_FAIL);
                    JCHK(NULL != buf->data,E_FAIL);
                    JCHK(0 < buf->size,E_FAIL);

                    uint8_t* pBuf = (uint8_t*)buf->data;
                    uint32_t szBuf = buf->size;
                    uint32_t offset = 0 ,len = 0;
                    get_pps_sps(pBuf,szBuf,AV_CODEC_ID_H264 == m_pStream->codec->codec_id,offset,len);
                    if(0 < len)
                    {
                        JCHK(NULL != (m_pStream->codec->extradata = (uint8_t*)av_realloc(m_pStream->codec->extradata,len)),E_OUTOFMEMORY);
                        memcpy(m_pStream->codec->extradata,pBuf+offset,len);
                        m_pStream->codec->extradata_size = len;
                    }
                }
            }
        }

        if(true == m_pMuxer->m_isGlobalHeader)
        {
            m_pStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            m_pStream->codec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
            if(AV_CODEC_ID_H264 == m_pStream->codec->codec_id || AV_CODEC_ID_HEVC == m_pStream->codec->codec_id)
            {
            }
            else if(AV_CODEC_ID_AAC == m_pStream->codec->codec_id)
            {
                if(false == m_isGlobalHeader)
                {
                    JCHK(m_ctxBSF = av_bitstream_filter_init("aac_adtstoasc"),E_FAIL);
                    m_preprocess = true;
                }
            }
        }
        else
        {
            m_pStream->codec->flags &= ~CODEC_FLAG_GLOBAL_HEADER;
            m_pStream->codec->flags2 |= AV_CODEC_FLAG2_LOCAL_HEADER;
            if(AV_CODEC_ID_H264 == m_pStream->codec->codec_id)
            {
                if(true == m_isGlobalHeader)
                {
                    JCHK(m_ctxBSF = av_bitstream_filter_init("h264_mp4toannexb"),E_FAIL);
                    m_preprocess = true;
                }
            }
            else if(AV_CODEC_ID_HEVC == m_pStream->codec->codec_id)
            {
                if(true == m_isGlobalHeader)
                {
                    JCHK(m_ctxBSF = av_bitstream_filter_init("hevc_mp4toannexb"),E_FAIL);
                    m_preprocess = true;
                }
            }
        }
        memset(&m_pktOut,0,sizeof(m_pktOut));
        m_pktOut.dts = AV_NOPTS_VALUE;
        m_pktOut.pts = AV_NOPTS_VALUE;
    }
    return hr;
}

HRESULT CFFmpegMuxer::CStream::Push(IMediaFrame* pFrame)
{
    if(NULL == pFrame)
    {
        m_isEnd = true;
    }
    else
    {
        if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
        {
            while(true == Pop()){}
        }
        m_frames.push_back(pFrame);
        m_isEnd = false;
    }
    return m_pMuxer->Write();
}

 HRESULT CFFmpegMuxer::CStream::Peek(IMediaFrame** ppFrame)
{
    if(m_frames.empty())
        return true == m_isEnd ? E_EOF : S_FALSE;
    else
        return m_frames.front().CopyTo(ppFrame);
}

bool CFFmpegMuxer::CStream::Pop()
{
    FrameIt it = m_frames.begin();
    if(it != m_frames.end())
    {
        m_frames.erase(it);
        return true;
    }
    else
        return false;
}

bool CFFmpegMuxer::CStream::IsEnd()
{
    return true == m_frames.empty() && true == m_isEnd;
}

HRESULT CFFmpegMuxer::CStream::Convert(AVPacket& pkt,IMediaFrame* pFrame)
{
    HRESULT hr;
    if(MMT_AUDIO == m_pMT->GetMajor())
    {
        //pFrame->info.dts += 20000000;
        //pFrame->info.pts += 20000000;
    }
    JIF(FrameToAVPacket(&pkt,pFrame,&m_pStream->time_base));
    pkt.stream_index = m_pStream->index;
    if(pkt.dts <= m_pktOut.dts)
    {
        ++pkt.dts;
        ++pkt.pts;
    }
    m_pktOut.dts = pkt.dts;
    m_pktOut.pts = pFrame->info.dts;
    if(true == m_preprocess)
    {
        if(AV_CODEC_ID_H264 == m_pStream->codec->codec_id || AV_CODEC_ID_HEVC == m_pStream->codec->codec_id)
        {
            if(false == m_pMuxer->m_isGlobalHeader)
            {
                if(NULL != m_pktOut.data)
                {
                    av_free(m_pktOut.data);
                    m_pktOut.data = NULL;
                }
                m_pktOut.size = 0;

                char err[AV_ERROR_MAX_STRING_SIZE] = {0};
                hr=av_bitstream_filter_filter(m_ctxBSF,
                            m_pStream->codec,
                            NULL,
                            &m_pktOut.data,
                            &m_pktOut.size,
                            pkt.data,
                            pkt.size,
                            pkt.flags & AV_PKT_FLAG_KEY);
                if(0 > hr)
                {
                    m_pktOut.data = NULL;
                    m_pktOut.size = 0;
                    LOG(1,"ffmpeg mux url:%s stream[%d] frame[DTS:%I64d] add header fail msg:%s",
                        m_pMuxer->m_name.c_str(),
                        m_pStream->index,
                        pkt.dts,
                        av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
                }
                else
                {
                    if(m_pktOut.data != pkt.data)
                    {
                        pkt.data = m_pktOut.data;
                        pkt.size = m_pktOut.size;
                    }
                    else
                    {
                        m_pktOut.data = NULL;
                        m_pktOut.size = 0;
                    }
                }
                hr = S_OK;
            }
        }
        else if(AV_CODEC_ID_AAC == m_pStream->codec->codec_id)
        {
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            hr = av_bitstream_filter_filter(m_ctxBSF,m_pStream->codec,NULL,
                &pkt.data,&pkt.size,pkt.data,pkt.size,pFrame->info.flag & AV_PKT_FLAG_KEY);
            if(0 > hr)
            {
                LOG(1,"ffmpeg muxer stream[%d] aac frame[DTS:%ld] romove header fail,msg:%s",
                    m_pStream->index,pFrame->info.dts,
                    av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
                hr = S_OK;
            }
        }
    }
    return hr;
}

HRESULT CFFmpegMuxer::CStream::Close()
{
    if(NULL != m_pktOut.data)
    {
        av_free(m_pktOut.data);
        m_pktOut.data = NULL;
    }
    m_pktOut.size = 0;
	if(NULL != m_ctxBSF)
	{
		av_bitstream_filter_close(m_ctxBSF);
		m_ctxBSF = NULL;
	}
    if(NULL != m_pStream->codec->extradata)
    {
        av_free(m_pStream->codec->extradata);
        m_pStream->codec->extradata = NULL;
    }
    m_pStream->codec->extradata_size = 0;
	if(NULL != m_pStream)
	{
        m_pStream->codec->extradata = NULL;
        m_pStream->codec->extradata_size = 0;
        m_pStream = NULL;
	}
    return S_OK;
}

bool CFFmpegMuxer::CStream::IsConnect()
{
    return m_spPin != NULL && NULL != m_spPin->GetConnection();
}
/*H264相关*/
CFFmpegMuxer::CFFmpegMuxer()
:m_ctxFormat(NULL)
,m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
,m_isFirst(true)
,m_isLive(false)
,m_isGlobalHeader(false)
,m_isImage(false)
,m_isImageWrite(false)
,m_isSegment(false)
,m_pos(MEDIA_FRAME_NONE_TIMESTAMP)
,m_dts(MEDIA_FRAME_NONE_TIMESTAMP)
{
    //ctor
    memset(&m_pkt,0,sizeof(m_pkt));
    m_base.num = 0;
    m_base.den = 1;
}

bool CFFmpegMuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    return true;
}

bool CFFmpegMuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
        StreamIt it;
        while(m_streams.end() != (it = m_streams.begin()))
        {
            CStream* pStream;
            if(NULL != (pStream = *it))
                delete pStream;
            m_streams.erase(it);
        }
        if(NULL != m_ctxFormat)
        {
            avformat_free_context(m_ctxFormat);
            m_ctxFormat = NULL;
        }
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegMuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFFmpegMuxer::GetType()
{
    return FT_Render;
}

STDMETHODIMP CFFmpegMuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegMuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegMuxer::GetFlag()
{
    return true == m_isLive ? FLAG_LIVE : 0;
}

STDMETHODIMP_(uint32_t) CFFmpegMuxer::GetInputPinCount()
{
    return m_streams.size();
}

STDMETHODIMP_(IInputPin*) CFFmpegMuxer::GetInputPin(uint32_t index)
{
    JCHK(index < m_streams.size(),NULL);
    return m_streams.at(index)->m_spPin;
}

STDMETHODIMP_(uint32_t) CFFmpegMuxer::GetOutputPinCount()
{
    return 0;
}

STDMETHODIMP_(IOutputPin*) CFFmpegMuxer::GetOutputPin(uint32_t index)
{
    return NULL;
}

STDMETHODIMP_(IInputPin*) CFFmpegMuxer::CreateInputPin(IMediaType* pMT)
{
    JCHK(NULL != pMT,NULL);
    JCHK(NULL != m_ctxFormat,NULL);

    MediaSubType sub;
    JCHK2(MST_NONE != (sub = GetFormatSubType(pMT->GetMajor())),NULL,
        "ffmpeg muxer format:%s is not support %s stream",
        m_ctxFormat->oformat->name,pMT->GetMajorName());

    MediaSubType sub_mt = pMT->GetSub();
    if(sub_mt != sub && false == QueryFormatSubType(sub_mt))
    {
        const char* name = pMT->GetSubLongName();
        pMT->SetSub(sub);
        LOG(0,"format:%s not support %s and change to %s",m_ctxFormat->oformat->name,name,pMT->GetSubLongName());
    }

    CStream* pStream;
    size_t index = m_streams.size();
    JCHK(pStream = new  CStream(this),NULL);
    JCHK(pStream->m_spPin.Create(CLSID_CInputPin,(IFilter*)this,false,&index),NULL);

    pStream->m_spMT = pMT;
    m_streams.push_back(pStream);
    return pStream->m_spPin;
}

STDMETHODIMP_(IOutputPin*) CFFmpegMuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFFmpegMuxer::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(S_Stop == m_status)
            {
                JIF(Open());
            }
            else if(S_Stop == cmd)
            {
                JIF(Close());
            }

            if(S_Play == cmd)
            {
                CStream* pStream;
                for(StreamIt it = m_streams.begin(); it != m_streams.end() ; ++it)
                {
                    if(NULL != (pStream = *it))
                    {
                        pStream->m_spPin->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                    }
                }
            }
            m_status = (Status)cmd;
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CFFmpegMuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFFmpegMuxer::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFFmpegMuxer::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegMuxer::GetExpend()
{
    return m_te.get();
}

STDMETHODIMP CFFmpegMuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    HRESULT hr = S_OK;

    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != m_ctxFormat,E_FAIL);

    CStream* pStream;
    size_t index = pPin->GetIndex();
    JCHK(pStream = m_streams[index],E_INVALIDARG);

    if(NULL != pMT)
    {
        if(pMT->GetSub() != pStream->m_spMT->GetSub())
            return E_INVALIDARG;
    }
    pStream->m_pMT = pMT;

    return hr;
}

STDMETHODIMP CFFmpegMuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CFFmpegMuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    uint32_t index = pPin->GetIndex();
    JCHK(index < m_streams.size(),E_INVALIDARG);
    return m_streams[index]->Push(pFrame);
}

STDMETHODIMP CFFmpegMuxer::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    HRESULT hr = S_OK;
    JCHK(NULL != pUrl,E_INVALIDARG);
    JCHK(FT_Render == mode,E_INVALIDARG);
    JCHK(false == m_isOpen,E_FAIL);
    if(NULL != m_ctxFormat)
    {
        CStream* pStream;
        for(StreamIt it = m_streams.begin(); it != m_streams.end() ; ++it)
        {
            if(NULL != (pStream = *it))
                pStream->m_pStream = NULL;
        }
        avformat_free_context(m_ctxFormat);
        m_ctxFormat = NULL;
    }
    const char* pFormat = NULL;
    CUrl url(pUrl);
    m_isLive = false;
    if(false != url.m_protocol.empty())
    {
        if(url.m_protocol == "rtmp")
        {
            pFormat = "flv";
            m_isLive = true;
        }
        else if(url.m_protocol == "http")
        {
            if(url.m_format  == "ts")
            {
                pFormat = "mpegts";
                m_isLive = true;
            }
        }
    }
    else
    {
        if(url.m_format  == "m3u8")
        {
            m_isLive = true;
        }
    }
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    JCHK2(0 <= (hr = avformat_alloc_output_context2(&m_ctxFormat,NULL,pFormat,pUrl)),
        E_FAIL,"ffmpeg muxer create url:[%s] fail,%s",pUrl,
        av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
    m_isGlobalHeader = 0 != (AVFMT_GLOBALHEADER & m_ctxFormat->oformat->flags);
    m_isImage = 0 == strcmp(m_ctxFormat->oformat->name,"image2");
    if(true == m_isImage)
    {
        m_ctxFormat->oformat->video_codec = av_guess_codec(m_ctxFormat->oformat,NULL,pUrl,NULL,AVMEDIA_TYPE_VIDEO);
    }
    else if(0 == strcmp(m_ctxFormat->oformat->name,"flv") ||
        0 == strcmp(m_ctxFormat->oformat->name,"mp4") ||
        0 == strcmp(m_ctxFormat->oformat->name,"mpegts"))
    {
        m_ctxFormat->oformat->video_codec = AV_CODEC_ID_H264;
        m_ctxFormat->oformat->audio_codec = AV_CODEC_ID_AAC;
    }
    return SetName(pUrl);
}

MediaSubType CFFmpegMuxer::GetFormatSubType(MediaMajorType major)
{
    MediaSubType sub = MST_NONE;
    if(MMT_VIDEO == major)
    {
        sub = (MediaSubType)m_ctxFormat->oformat->video_codec;
    }
    else if(MMT_AUDIO == major)
    {
        sub = (MediaSubType)m_ctxFormat->oformat->audio_codec;
    }
    else if(MMT_SUBTITLE == major)
    {
        sub = (MediaSubType)m_ctxFormat->oformat->subtitle_codec;
    }
    return sub;
}

bool CFFmpegMuxer::QueryFormatSubType(MediaSubType sub)
{
    if(true == m_isImage)
    {
        return sub == (MediaSubType)m_ctxFormat->oformat->video_codec;
    }
    else
    {
        if(1 == avformat_query_codec(m_ctxFormat->oformat,(AVCodecID)sub,FF_COMPLIANCE_UNOFFICIAL))
            return true;
        else if(0 == strcmp("mpegts",m_ctxFormat->oformat->name))
        {
            if(sub == MST_H264 || sub == MST_HEVC || sub == MST_AAC)
                return true;
        }
    }
    return false;
}

HRESULT CFFmpegMuxer::Open()
{
    HRESULT hr = S_OK;

    if(true == m_isOpen)
        return hr;

    JCHK(false == m_name.empty(),E_FAIL);
    if(NULL != m_ctxFormat)
    {
        avformat_free_context(m_ctxFormat);
        m_ctxFormat = NULL;
    }
    JIF(Load(m_name.c_str(),FT_Render));
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    GetOption(m_ctxFormat->priv_data,m_spProfile);
    if(0 == strcmp(m_ctxFormat->oformat->name,"hls"))
    {
        m_isLive = true;
        int hls_list_size = 0;
        if(true == m_spProfile->Read("hls_list_size",hls_list_size))
        {
            if(0 == hls_list_size)
                m_isLive = false;
        }
    }

    m_ctxFormat->interrupt_callback.callback = timeout_callback;
    m_ctxFormat->interrupt_callback.opaque = this;
    JCHK2(0 <= (hr = avio_open2(&m_ctxFormat->pb,m_name.c_str(),AVIO_FLAG_WRITE,true == m_isLive ? &m_ctxFormat->interrupt_callback : NULL,NULL)),
        E_FAIL,"ffmpeg muxer connect url:[%s] fail,msg::%s",m_name.c_str(),av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
    av_dump_format(m_ctxFormat,0,m_name.c_str(),1);

    CStream* pStream;
    m_base.num = 0;
    m_base.den = 1;

    for(StreamIt it = m_streams.begin(); it != m_streams.end() ; ++it)
    {
        if(NULL != (pStream = *it))
        {
            JIF(pStream->Open());
        }
    }

    for(StreamIt it = m_streams.begin(); it != m_streams.end() ; ++it)
    {
        if(NULL != (pStream = *it))
        {
            pStream->m_pStream->time_base = m_base;
            pStream->m_pStream->codec->time_base = m_base;
        }
    }
    av_init_packet(&m_pkt);
    m_isSegment = true;
    m_isFirst = true;
    m_isOpen = true;
    return hr;
}

HRESULT CFFmpegMuxer::Close()
{
    HRESULT hr = S_OK;
    if(false == m_isFirst)
    {
        char err[AV_ERROR_MAX_STRING_SIZE] = {0};
		hr = av_write_trailer(m_ctxFormat);
        if(0 != hr)
        {
            LOG(1,"ffmpeg muxer write trailer fail,url:[%s],msg::%s",m_name.c_str(),
            av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
        }
        m_isFirst = true;
    }
    if(true == m_isOpen)
    {
        if(NULL != m_ctxFormat)
        {
            CStream* pStream;
            for(StreamIt it = m_streams.begin(); it != m_streams.end() ; ++it)
            {
                if(NULL != (pStream = *it))
                {
                    pStream->Close();
                }
            }
            avio_closep(&m_ctxFormat->pb);
        }
        m_isOpen = false;
    }
    return hr;
}

HRESULT CFFmpegMuxer::Write()
{
    HRESULT hr;
    do
    {
        bool segment = true;
        CStream* pStreamResult = NULL;
        dom_ptr<IMediaFrame> spFrameResult;

        for(uint32_t i=0 ;i<m_streams.size() ; ++i)
        {
            dom_ptr<IMediaFrame> spFrame;
            CStream* pStream = m_streams[i];
            JIF(pStream->Peek(&spFrame));
            if(S_OK == hr)
            {
                if(spFrameResult == NULL || spFrame->info.dts <= spFrameResult->info.dts)
                {
                    pStreamResult = pStream;
                    spFrameResult = spFrame;
                }
                if(0 == (spFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT) && true == segment)
                    segment = false;
            }
            else if(S_FALSE == hr)
                return S_OK;
        }
        if(spFrameResult != NULL)
            spFrameResult->info.segment = segment;
        hr = WriteFrame(pStreamResult,spFrameResult);
        if(NULL != pStreamResult)
            pStreamResult->Pop();
    }while(S_OK <= hr && E_EOF != hr);
    return hr;
}

HRESULT CFFmpegMuxer::WriteFrame(CStream* pStream,IMediaFrame* pFrame)
{
	if(NULL == pFrame)
	{
        Close();
        return E_EOF;
	}

    HRESULT hr = S_OK;
    JIF(m_ep->Notify(ET_Filter_Render,hr,pStream->m_spPin.p,pFrame));

    if(true == m_isImage)
    {
        if(m_name == m_ctxFormat->filename)
        {
            if(true == m_isImageWrite)
            {
                return S_FALSE;
            }
        }
        else
        {
            m_isImageWrite = false;
        }
    }


    time_expend te(m_te);

    JUMP(S_OK == Open(),S_FALSE);

    if(true == m_isSegment)
    {
        if(false == pFrame->info.segment)
            return S_FALSE;
        m_isSegment = false;
    }

    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    if(true == m_isFirst)
    {
		hr = avformat_write_header(m_ctxFormat,NULL);
        JCHK2(0 <= hr,S_FALSE,
            "ffmpeg muxer write file[%s] header fail,msg:%s",m_name.c_str(),
            av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
        m_isFirst = false;
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    JUMP(S_OK == pStream->Convert(pkt,pFrame),S_FALSE);

//    if(MMT_AUDIO == pStream->m_pMT->GetMajor())
//    {
//        printf("stream[%d:%s-%s] write frame DTS:%ldms PTS:%ldms pkt DTS:%ld PTS:%ld size:%d \n",
//            pStream->m_pStream->index,pStream->m_pMT->GetMajorName(),pStream->m_pMT->GetSubName(),
//            pFrame->info.dts/10000,pFrame->info.pts/10000,pkt.dts,pkt.pts,pkt.size);
//    }
//    if(pkt.dts <= m_pkt.dts)
//    {
//        int64_t delta = m_pkt.dts + 1 - pkt.dts;
//        pkt.dts += delta;
//        pkt.pts += delta;
//    }
//    m_pkt = pkt;
    if(pkt.dts > pkt.pts)
    {
        LOG(1,"stream[%d:%s-%s] write packet PTS:%ld < DTS:%ld",
            pStream->m_pStream->index,pStream->m_pMT->GetMajorName(),
            pStream->m_pMT->GetSubName(),pkt.pts,pkt.dts);
        pkt.pts = pkt.dts;
    }
    hr = av_write_frame(m_ctxFormat,&pkt);
//    if(true == m_pFG->IsExit())
//        return S_OK;

    if(S_OK > hr)
    {
        Close();
        JCHK4(false,S_FALSE,
            "ffmpeg muxer stream:%d write packet DTS:%dms PTS:%dms fail:%s",
            pkt.stream_index,int(pFrame->info.dts/10000),int(pFrame->info.pts/10000),
            av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
    }

    if(true == m_isImage)
    {
        Close();
        m_isImageWrite = true;
    }
    return S_OK;
}

int CFFmpegMuxer::timeout_callback(void *pParam)
{
//	CFFmpegMuxer* pThis = (CFFmpegMuxer*)pParam;
//	return true == pThis->m_pFG->IsExit() ? 1 : 0;
    return false;
}

HRESULT CFFmpegMuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL == pMtOut)
        return 0;
    return E_INVALIDARG;
}

