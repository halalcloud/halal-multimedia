#include "FFmpegVideoDecoder.h"
CFFmpegVideoDecoder::CFFmpegVideoDecoder()
:m_pFG(NULL)
,m_isOpen(false)
,m_pTag(NULL)
,m_pMtIn(NULL)
,m_pMtOut(NULL)
,m_ctxCodec(NULL)
,m_pFrame(NULL)
,m_duration(0)
,m_width(0)
,m_height(0)
{
    //ctor
	av_init_packet(&m_pkt);
}

bool CFFmpegVideoDecoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
    return true;
}

bool CFFmpegVideoDecoder::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegVideoDecoder)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CFFmpegVideoDecoder::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegVideoDecoder::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegVideoDecoder::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoDecoder::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegVideoDecoder::GetInputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinIn;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoDecoder::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegVideoDecoder::GetOutputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinOut;
}

STDMETHODIMP CFFmpegVideoDecoder::Open()
{
    HRESULT hr = S_OK;
    JCHK(NULL != m_pMtIn,E_FAIL);
    JCHK(NULL != m_pMtOut,E_FAIL);
    if(false == m_isOpen)
    {
        m_ctxCodec->thread_count = 0;
        JCHK1(0 == avcodec_open2(m_ctxCodec,avcodec_find_decoder(m_ctxCodec->codec_id),NULL),E_FAIL,"Open media[%s] decoder fail",m_pMtIn->GetSubLongName());
        JCHK(m_pFrame = av_frame_alloc(),E_FAIL);
        av_init_packet(&m_pkt);
        m_pkt.pos = AV_NOPTS_VALUE;
        m_dts = MEDIA_FRAME_NONE_TIMESTAMP;
        m_isOpen = true;
    }
	return hr;
}

STDMETHODIMP CFFmpegVideoDecoder::Close()
{
    if(true == m_isOpen)
    {
		if(NULL != m_pFrame)
			av_frame_free(&m_pFrame);
        avcodec_close(m_ctxCodec);
        m_isOpen = false;
    }
    return S_OK;
}

STDMETHODIMP CFFmpegVideoDecoder::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CFFmpegVideoDecoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegVideoDecoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegVideoDecoder::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CFFmpegVideoDecoder::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    if(NULL == m_pMtIn || NULL == m_ctxCodec)
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

STDMETHODIMP CFFmpegVideoDecoder::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
		AVCodec* pCodec;
		JIF(CheckMediaType(pMT,m_pMtOut));
        AVCodecID id = (AVCodecID)pMT->GetSub();
		JCHK1(pCodec = avcodec_find_decoder(id),E_INVALIDARG,
            "media[%s] can not find decoder",pMT->GetSubLongName());
        JCHK0(m_ctxCodec = avcodec_alloc_context3(pCodec),E_FAIL,"Create decoder ctx fail");

        m_ctxCodec->pix_fmt = (AVPixelFormat)VMT_YUV420P;
        pMT->GetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,
            &m_ctxCodec->width,
            &m_ctxCodec->height,
            &m_ratioX,
            &m_ratioY,
            &m_duration);
        m_width = m_ctxCodec->width;
        m_height = m_ctxCodec->height;
        m_ctxCodec->framerate.num = FRAME_TIMEBASE.den;
        m_ctxCodec->framerate.den = m_duration;
        m_ctxCodec->time_base = FRAME_TIMEBASE;
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

        if(1 > m_ctxCodec->block_align)
            m_ctxCodec->block_align = VIDEO_ALIGN;

        bool isGlobalHeader = false;
        pMT->GetStreamInfo(NULL,NULL,&m_ctxCodec->bit_rate,&isGlobalHeader,&m_ctxCodec->extradata,&m_ctxCodec->extradata_size);
        if(true == isGlobalHeader)
        {
            m_ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            m_ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
        }
        else
        {
            m_ctxCodec->flags &= ~CODEC_FLAG_GLOBAL_HEADER;
            m_ctxCodec->flags2 |= AV_CODEC_FLAG2_LOCAL_HEADER;
        }
    }
    else
    {
        if(NULL != m_ctxCodec)
        {
            av_free(m_ctxCodec);
            m_ctxCodec = NULL;
        }
    }
    m_pMtIn = pMT;
    return hr;
}

STDMETHODIMP CFFmpegVideoDecoder::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        JIF(CheckMediaType(m_pMtIn,pMT));

        JIF(pMT->GetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,&m_ctxCodec->width,
            &m_ctxCodec->height,&m_ratioX,&m_ratioY,&m_duration));

        m_ctxCodec->framerate.num = FRAME_TIMEBASE.den;
        m_ctxCodec->framerate.den = m_duration;
        m_ctxCodec->time_base = VIDEO_TIMEBASE;

    }
    m_pMtOut = pMT;
    return hr;
}

STDMETHODIMP CFFmpegVideoDecoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
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

STDMETHODIMP CFFmpegVideoDecoder::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

HRESULT CFFmpegVideoDecoder::Write(IMediaFrame* pFrame)
{
	HRESULT hr;
	if(pFrame == NULL)
	{
		m_pkt.buf = NULL;
		m_pkt.data = NULL;
		m_pkt.size = 0;
	}
	else
	{
		if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
		{
			avcodec_flush_buffers(m_ctxCodec);
			m_spPinOut->NewSegment();
		}

        if(MEDIA_FRAME_NONE_TIMESTAMP == m_dts)
        {
            if(0 == (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
            {
                LOG(4,"video decoder first input frame[DTS:%dms] is not key frame and will be dropped",pFrame->info.dts/10000);
                return S_FALSE;
            }
        }
        else if(pFrame->info.dts <= m_dts)
        {
            LOG(4,"video decoder input previous[DTS:%dms] current[DTS:%dms]",m_dts/10000,pFrame->info.dts/10000);
        }

		m_dts = pFrame->info.dts;

        //printf("\nvideo decoder in packet[DTS:%d,PTS:%d] time base:[%d,%d]\n",m_pkt.dts,m_pkt.pts,m_ctxCodec->time_base.num,m_ctxCodec->time_base.den);
		JCHK(S_OK == FrameToAVPacket(&m_pkt,pFrame),S_FALSE);
		if(AV_NOPTS_VALUE == m_pkt.pos || m_pkt.pts > m_pkt.pos)
		{
            m_pkt.pos = m_pkt.pts;
		}
	}

	int isOutput = 0;
	//printf("Decode in DTS:%dms PTS:%dms flag:%d\n",int(pFrame->info.dts/10000),int(pFrame->info.pts/10000),pFrame->info.flag);
	hr = avcodec_decode_video2(m_ctxCodec,m_pFrame, &isOutput,&m_pkt);
	if(0 != isOutput)
	{
        if(m_pFrame->pkt_dts != m_pFrame->pkt_pts)
            m_pFrame->pkt_dts = m_pFrame->pkt_pts;

        dom_ptr<IMediaFrame> spFrame;
        JIF(m_spPinOut->AllocFrame(&spFrame));

        if(m_pFrame->width != m_width || m_pFrame->height != m_height)
        {
            m_width = m_pFrame->width;
            m_height = m_pFrame->height;
            JIF(m_pMtOut->SetVideoInfo(NULL,&m_width,&m_height))
            spFrame->info.flag |= MEDIA_FRAME_FLAG_MEDIA_CHANGE;
        }
		JIF(AVFrameToFrame(m_pMtOut,spFrame,m_pFrame));
        //printf("------------------Decode out DTS:%dms PTS:%dms\n",int(spFrame->info.dts/10000),int(spFrame->info.pts/10000));
        hr = m_spPinOut->Write(spFrame);

	}
	else
	{
        if(NULL == pFrame)
        {
            hr =  S_STREAM_EOF;
        }
        else if(0 > hr)
        {
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            JCHK2(0 <= hr,S_FALSE,"ffmpeg video decode frame[DTS:%dms] fail,msg:%s"
                ,pFrame->info.dts/10000,av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr))
        }
        else
            hr = S_OK;
	}
    return hr;
}

HRESULT CFFmpegVideoDecoder::CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr = S_OK;

	if(NULL == pMtIn)
		return E_INVALIDARG;

	if(MMT_VIDEO != pMtIn->GetMajor())
        return E_INVALIDARG;

	AVCodec* pCodec;
	MediaSubType sub = pMtIn->GetSub();
	if(sub == MST_RAWVIDEO || NULL == (pCodec = avcodec_find_decoder((AVCodecID)sub)))
		return E_INVALIDARG;

    int width_in = 0,height_in = 0;
    int64_t duration_in = 0;

    JIF(pMtIn->GetVideoInfo(NULL,&width_in,&height_in,NULL,NULL,&duration_in));

	if(NULL != pMtOut)
	{
        if(MST_RAWVIDEO != pMtOut->GetSub())
            return E_INVALIDARG;

        AVPixelFormat pix_fmt_out = AV_PIX_FMT_NONE;
        int width_out = 0,height_out = 0;
        int64_t duration_out = 0;
        JIF(pMtOut->GetVideoInfo((VideoMediaType*)&pix_fmt_out,&width_out,&height_out,NULL,NULL,&duration_out));

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
        else if(duration_in != duration_out)
            return E_INVALIDARG;

        if(width_in != width_out)
            return E_INVALIDARG;

        if(height_in != height_out)
            return E_INVALIDARG;
	}
	return hr;
}

