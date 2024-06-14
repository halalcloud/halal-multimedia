#include "FFmpegVideoEncoder.h"

CFFmpegVideoEncoder::CFFmpegVideoEncoder()
:m_pFG(NULL)
,m_isOpen(false)
,m_pTag(NULL)
,m_pMtIn(NULL)
,m_pMtOut(NULL)
,m_ctxCodec(NULL)
,m_duration(0)
,m_ratioX(0)
,m_ratioY(0)
{
}

bool CFFmpegVideoEncoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,dynamic_cast<IFilter*>(this)),false);
    return true;
}

bool CFFmpegVideoEncoder::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegVideoEncoder)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CFFmpegVideoEncoder::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegVideoEncoder::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegVideoEncoder::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoEncoder::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegVideoEncoder::GetInputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinIn;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoEncoder::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegVideoEncoder::GetOutputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinOut;
}

STDMETHODIMP CFFmpegVideoEncoder::Open()
{
    HRESULT hr = S_OK;
    JCHK(NULL != m_pMtIn,E_FAIL);
    JCHK(NULL != m_pMtOut,E_FAIL);
    if(false == m_isOpen)
    {
        bool isGlobalHeader = false;
        m_pMtOut->GetStreamInfo(NULL,NULL,&m_ctxCodec->bit_rate,NULL,NULL,NULL);
        m_ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        m_ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
        if(AV_CODEC_ID_MJPEG != m_ctxCodec->codec_id)
        {
            m_ctxCodec->rc_min_rate =m_ctxCodec->bit_rate;
            m_ctxCodec->rc_max_rate = m_ctxCodec->bit_rate;
            m_ctxCodec->bit_rate_tolerance = m_ctxCodec->bit_rate;
            m_ctxCodec->rc_buffer_size = m_ctxCodec->bit_rate;
            m_ctxCodec->rc_initial_buffer_occupancy = m_ctxCodec->rc_buffer_size*3/4;
        }

        char err[AV_ERROR_MAX_STRING_SIZE] = {0};
        m_ctxCodec->thread_count = 0;
        JCHK2(0 == (hr = avcodec_open2(m_ctxCodec,avcodec_find_encoder(m_ctxCodec->codec_id),NULL)),E_FAIL,
            "Open media[%s] encoder fail msg:%s",m_pMtOut->GetSubLongName(),
            av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

        dom_ptr<IProfile> spProfile(this);
        GetOption(m_ctxCodec->priv_data,spProfile);
        JIF(m_pMtOut->SetStreamInfo(NULL,NULL,&m_ctxCodec->bit_rate,&isGlobalHeader,m_ctxCodec->extradata,&m_ctxCodec->extradata_size));
        m_isOpen = true;
    }
	return hr;
}

STDMETHODIMP CFFmpegVideoEncoder::Close()
{
    if(true == m_isOpen)
    {
        avcodec_close(m_ctxCodec);
        m_isOpen = false;
    }
    return S_OK;
}

STDMETHODIMP CFFmpegVideoEncoder::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CFFmpegVideoEncoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegVideoEncoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegVideoEncoder::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    if(NULL == m_pMtOut || NULL == m_ctxCodec)
        return E_FAIL;
    else
    {
        HRESULT hr;
        JIF(pMT->Clear());
        JIF(pMT->SetSub(MST_RAWVIDEO));
        JIF(pMT->SetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,
            &m_ctxCodec->width,
            &m_ctxCodec->height,
            &m_ratioX,
            &m_ratioY,
            &m_duration));
        return hr;
    }
}

STDMETHODIMP CFFmpegVideoEncoder::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CFFmpegVideoEncoder::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;

    if(NULL != pMT)
    {
        JIF(CheckMediaType(pMT,m_pMtOut));
        JIF(pMT->GetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,&m_ctxCodec->width,
            &m_ctxCodec->height,&m_ratioX,&m_ratioY,&m_duration));
        m_ctxCodec->framerate.num = FRAME_TIMEBASE.den;
        m_ctxCodec->framerate.den = m_duration;
        m_ctxCodec->time_base.num = m_duration;
        m_ctxCodec->time_base.den = FRAME_TIMEBASE.den;
        if(0 == m_ratioX)
            m_ratioX = m_ctxCodec->width;
        if(0 == m_ratioY)
            m_ratioY = m_ctxCodec->height;

        unsigned int g = gcd(m_ratioX,m_ratioY);
        m_ratioX /= g;
        m_ratioY /= g;

        unsigned int sar_x = m_ratioX * m_ctxCodec->height;
        unsigned int sar_y = m_ratioY * m_ctxCodec->width;

        g = gcd(sar_x,sar_y);
        m_ctxCodec->sample_aspect_ratio.num = sar_x/g;
        m_ctxCodec->sample_aspect_ratio.den = sar_y/g;
    }
    m_pMtIn = pMT;
    return hr;
}

STDMETHODIMP CFFmpegVideoEncoder::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
		AVCodec* pCodec;
        JIF(CheckMediaType(m_pMtIn,pMT));
        AVCodecID id = (AVCodecID)pMT->GetSub();
		JCHK1(pCodec = avcodec_find_encoder(id),E_INVALIDARG,
            "media[%s] can not find encoder",pMT->GetSubLongName());
        JCHK0(m_ctxCodec = avcodec_alloc_context3(pCodec),E_FAIL,"Create encoder ctx fail");

        JIF(pMT->GetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,
            &m_ctxCodec->width,&m_ctxCodec->height,&m_ratioX,&m_ratioY,&m_duration));

        if(NULL != pCodec->pix_fmts)
        {
            int i=0;
            while(AV_PIX_FMT_NONE != pCodec->pix_fmts[i] && m_ctxCodec->pix_fmt != pCodec->pix_fmts[i]){++i;}
            if(AV_PIX_FMT_NONE == pCodec->pix_fmts[i])
                m_ctxCodec->pix_fmt = pCodec->pix_fmts[0];
        }

        if(NULL != pCodec->supported_framerates)
        {
            GetVideoDuration(pCodec->supported_framerates,m_duration);
        }

        m_ctxCodec->framerate.num = FRAME_TIMEBASE.den;
        m_ctxCodec->framerate.den = m_duration;
        m_ctxCodec->time_base.num = m_duration;
        m_ctxCodec->time_base.den = FRAME_TIMEBASE.den;

        m_ctxCodec->sample_aspect_ratio.num = m_ratioX * m_ctxCodec->height;
        m_ctxCodec->sample_aspect_ratio.den = m_ratioY * m_ctxCodec->width;

        int g = gcd(m_ctxCodec->sample_aspect_ratio.num,m_ctxCodec->sample_aspect_ratio.den);
        if(1 < g)
        {
            m_ctxCodec->sample_aspect_ratio.num /= g;
            m_ctxCodec->sample_aspect_ratio.den /= g;
        }
        JIF(pMT->SetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,
            &m_ctxCodec->width,&m_ctxCodec->height,&m_ratioX,&m_ratioY,&m_duration));
        bool isGlobalHeader = false;
        JIF(pMT->SetStreamInfo(NULL,NULL,NULL,&isGlobalHeader));
    }
    else
    {
        if(NULL != m_ctxCodec)
        {
            av_free(m_ctxCodec);
            m_ctxCodec = NULL;
        }
    }
    m_pMtOut = pMT;
    return hr;
}

STDMETHODIMP CFFmpegVideoEncoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    HRESULT hr;
    if(false == m_isOpen)
    {
        if(NULL != pFrame)
        {
            JIF(Open());
        }
    }
    if(true == m_isOpen)
    {
        hr = Write(pFrame);
        if(S_STREAM_EOF == hr)
            Close();
    }
    if(false == m_isOpen)
        hr = m_spPinOut->Write(pFrame);
    return hr;
}

STDMETHODIMP CFFmpegVideoEncoder::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

HRESULT CFFmpegVideoEncoder::Write(IMediaFrame* pFrame)
{
	HRESULT hr = S_OK;
	AVFrame frame;
	AVFrame* pFrameIn = NULL;
	if(NULL != pFrame)
	{
		if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
		{
			avcodec_flush_buffers(m_ctxCodec);
			m_spPinOut->NewSegment();
		}

        hr = m_pFG->Notify(IFilterEvent::Process,hr,this,m_spPinIn,m_spPinOut,pFrame);
        if(S_OK != hr)
            return hr;
		memset(&frame,0,sizeof(frame));
		frame.extended_data = frame.data;
		JIF(FrameToAVFrame(m_pMtIn,&frame,pFrame,m_ctxCodec));
		pFrameIn = &frame;
        //printf("encode input frame DTS:%ld PTS:%ld\n",pFrame->info.dts/10000,pFrame->info.pts/10000);
	}
    AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	int isOutput = 0;
	hr = avcodec_encode_video2(m_ctxCodec,&pkt,pFrameIn,&isOutput);
	if(0 != isOutput)
	{
        dom_ptr<IMediaFrame> spFrame;
        JIF(m_spPinOut->AllocFrame(&spFrame));
		JCHK(S_OK == AVPacketToFrame(spFrame,pkt,m_ctxCodec->time_base,m_ctxCodec->frame_size),S_FALSE);
        //printf("encode output frame \tDTS:%ld PTS:%ld\n",spFrame->info.dts/10000,spFrame->info.pts/10000);
		hr = m_spPinOut->Write(spFrame);
        av_packet_unref(&pkt);
	}
	else
	{
        if(NULL == pFrame)
            hr = S_STREAM_EOF;
        else if(0 > hr)
        {
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            JCHK2(0 == hr,S_FALSE,"ffmpeg encode frame[DTS:%I64d] fail,msg:%s"
                ,pFrame->info.dts,av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr))
        }
        else
            hr = S_OK;
	}
	return hr;
}

HRESULT CFFmpegVideoEncoder::CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr = S_OK;

    if(NULL == pMtOut)
        return E_INVALIDARG;

	if(MMT_VIDEO != pMtOut->GetMajor())
        return E_INVALIDARG;

    AVCodec* pCodec;
	MediaSubType sub = pMtOut->GetSub();
	if(sub == MST_RAWVIDEO || NULL == (pCodec = avcodec_find_encoder((AVCodecID)sub)))
		return E_INVALIDARG;

    AVPixelFormat pix_fmt_out = AV_PIX_FMT_NONE;
    int width_out = 0,height_out = 0;
    int64_t duration_out = 0;

    JIF(pMtOut->GetVideoInfo((VideoMediaType*)&pix_fmt_out,&width_out,&height_out,NULL,NULL,&duration_out));

	if(NULL != pMtIn)
	{

        if(MST_RAWVIDEO != pMtIn->GetSub())
            return E_INVALIDARG;

        AVPixelFormat pix_fmt_in = AV_PIX_FMT_NONE;
        int width_in = 0,height_in = 0;
        int64_t duration_in = 0;
        JIF(pMtIn->GetVideoInfo((VideoMediaType*)&pix_fmt_in,&width_in,&height_in,NULL,NULL,&duration_in));

        if(pix_fmt_in != pix_fmt_out)
            return E_INVALIDARG;

        if(width_in != width_out)
            return E_INVALIDARG;

        if(height_in != height_out)
            return E_INVALIDARG;

        if(duration_in != duration_out)
            return E_INVALIDARG;

        if(NULL != pCodec->pix_fmts)
        {
            int i=0;
            while(AV_PIX_FMT_NONE != pCodec->pix_fmts[i] && pix_fmt_out != pCodec->pix_fmts[i]){++i;}
            if(AV_PIX_FMT_NONE == pCodec->pix_fmts[i])
                return E_INVALIDARG;
        }

        if(NULL != pCodec->supported_framerates)
        {
            if(false == GetVideoDuration(pCodec->supported_framerates,duration_out))
                return E_INVALIDARG;
        }
	}
	return hr;
}

