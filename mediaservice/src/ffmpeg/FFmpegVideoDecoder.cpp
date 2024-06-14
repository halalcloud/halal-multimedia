#include "FFmpegVideoDecoder.h"
CFFmpegVideoDecoder::CFFmpegVideoDecoder()
:m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
,m_ctxCodec(NULL)
,m_pFrame(NULL)
,m_duration(0)
,m_width(0)
,m_height(0)
,m_ratioX(0)
,m_ratioY(0)
{
    //ctor
	av_init_packet(&m_pkt);
}

bool CFFmpegVideoDecoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,(IFilter*)this),false);
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
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFFmpegVideoDecoder::GetType()
{
    return FT_Transform;
}

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
    return m_spPinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegVideoDecoder::GetInputPin(uint32_t index)
{
    return 0 == index ? m_spPinIn : NULL;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoDecoder::GetOutputPinCount()
{
    return m_spPinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegVideoDecoder::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_spPinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CFFmpegVideoDecoder::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFFmpegVideoDecoder::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFFmpegVideoDecoder::Notify(uint32_t cmd)
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
                m_spPinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                m_spPinOut->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
            }
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    else if(cmd < C_NB)
    {
        if(C_Flush == cmd)
        {
            do
            {
                hr = Write(NULL);
            }while(hr >= S_OK);
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoDecoder::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFFmpegVideoDecoder::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFFmpegVideoDecoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegVideoDecoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegVideoDecoder::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        JIF(FilterQuery(NULL,NULL,pMT,m_spPinOut == NULL ? NULL : m_spPinOut->GetMediaType()));
		AVCodec* pCodec;
        AVCodecID id = (AVCodecID)pMT->GetSub();
		JCHK1(pCodec = avcodec_find_decoder(id),E_INVALIDARG,
            "media[%s] can not find decoder",pMT->GetSubLongName());
        JCHK0(m_ctxCodec = avcodec_alloc_context3(pCodec),E_FAIL,"Create decoder ctx fail");
        if(AV_PIX_FMT_NONE == m_ctxCodec->pix_fmt)
        {
            if(NULL == m_ctxCodec->codec->pix_fmts)
                m_ctxCodec->pix_fmt = (AVPixelFormat)VMT_YUV420P;
            else
                m_ctxCodec->pix_fmt = m_ctxCodec->codec->pix_fmts[0];
        }

        m_ctxCodec->time_base = FRAME_TIMEBASE;
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

        if(S_OK == pMT->GetVideoInfo(NULL,
            &m_width,
            &m_height,
            &m_ratioX,
            &m_ratioY,
            &m_duration))
        {
            m_ctxCodec->width = m_width;
            m_ctxCodec->height = m_height;
            m_ctxCodec->framerate.num = FRAME_TIMEBASE.den;
            m_ctxCodec->framerate.den = m_duration;
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

            if(m_spPinOut == NULL)
            {
                dom_ptr<IMediaType> spMT;
                JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
                JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
                JIF(spMT->SetSub(MST_RAWVIDEO));
                JIF(spMT->SetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,
                    &m_ctxCodec->width,
                    &m_ctxCodec->height,
                    &m_ratioX,
                    &m_ratioY,
                    &m_duration));
                JIF(m_spPinOut->SetMediaType(spMT));
            }
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
    return hr;
}

STDMETHODIMP CFFmpegVideoDecoder::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        JIF(FilterQuery(NULL,NULL,m_spPinIn->GetMediaType(),pMT));
        AVPixelFormat pmt;
        JIF(pMT->GetVideoInfo((VideoMediaType*)&pmt,&m_ctxCodec->width,
            &m_ctxCodec->height,&m_ratioX,&m_ratioY,&m_duration));
        if(NULL == m_ctxCodec->codec->pix_fmts)
        {
            if(m_ctxCodec->pix_fmt != pmt)
                return E_INVALIDARG;
        }
        m_ctxCodec->framerate.num = FRAME_TIMEBASE.den;
        m_ctxCodec->framerate.den = m_duration;
        m_ctxCodec->time_base = VIDEO_TIMEBASE;
    }
    return hr;
}

STDMETHODIMP CFFmpegVideoDecoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Write(pFrame);
}

HRESULT CFFmpegVideoDecoder::Open()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
    {
        m_ctxCodec->thread_count = 0;
        JCHK0(0 == avcodec_open2(m_ctxCodec,avcodec_find_decoder(m_ctxCodec->codec_id),NULL),E_FAIL,"Open decoder fail");
        JCHK(m_pFrame = av_frame_alloc(),E_FAIL);
        av_init_packet(&m_pkt);
        m_pkt.pos = AV_NOPTS_VALUE;
        m_dts = MEDIA_FRAME_NONE_TIMESTAMP;
        m_isOpen = true;
    }
	return hr;
}

HRESULT CFFmpegVideoDecoder::Close()
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
	    if(m_spPinOut == NULL)
        {
            IMediaType* pMtIn;
            JCHK(pMtIn = m_spPinIn->GetMediaType(),E_FAIL);
            if(0 >= (m_width = m_ctxCodec->width))
                return S_OK;
            if(0 >= (m_height = m_ctxCodec->height))
                return S_OK;
            if(m_ctxCodec->framerate.den != 0 && m_ctxCodec->framerate.num != 0)
                m_duration = int64_t(10000000/av_q2d(m_ctxCodec->framerate)+0.5);
            else
                return S_OK;

            JIF(pMtIn->SetVideoInfo(NULL,&m_ctxCodec->width,&m_ctxCodec->height,NULL,NULL,&m_duration));

            m_ctxCodec->width = m_width;
            m_ctxCodec->height = m_height;
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

            dom_ptr<IMediaType> spMT;
            JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMT->SetSub(MST_RAWVIDEO));
            JIF(spMT->SetVideoInfo((VideoMediaType*)&m_ctxCodec->pix_fmt,
                &m_ctxCodec->width,
                &m_ctxCodec->height,
                &m_ratioX,
                &m_ratioY,
                &m_duration));
            JIF(m_spPinOut->SetMediaType(spMT));
            JIF(m_ep->Notify(ET_Filter_Build,0,(IFilter*)this));
        }
        IMediaType* pMtOut;
        JCHK(m_spPinOut != NULL,E_FAIL);
        JCHK(NULL != (pMtOut = m_spPinOut->GetMediaType()),E_FAIL);

        if(m_pFrame->pkt_dts != m_pFrame->pkt_pts)
            m_pFrame->pkt_dts = m_pFrame->pkt_pts;

        dom_ptr<IMediaFrame> spFrame;
        JIF(m_spPinOut->AllocFrame(&spFrame));

        if(m_pFrame->width != m_width || m_pFrame->height != m_height)
        {
            m_width = m_pFrame->width;
            m_height = m_pFrame->height;
            JIF(pMtOut->SetVideoInfo(NULL,&m_width,&m_height))
            spFrame->info.flag |= MEDIA_FRAME_FLAG_MEDIA_CHANGE;
        }

		JIF(AVFrameToFrame(pMtOut,spFrame,m_pFrame));
//        printf("------------------Decode out DTS:%ld PTS:%ld\n",spFrame->info.dts,spFrame->info.pts);
        hr = m_spPinOut->Write(spFrame);

	}
	else
	{
        if(NULL == pFrame)
        {
            hr =  E_EOF;
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
HRESULT CFFmpegVideoDecoder::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
	if(NULL == pMtIn)
		return E_INVALIDARG;

	if(MMT_VIDEO != pMtIn->GetMajor())
        return E_INVALIDARG;

    HRESULT hr = S_OK;

	AVCodec* pCodec;
	MediaSubType sub = pMtIn->GetSub();
	if(sub == MST_RAWVIDEO || NULL == (pCodec = avcodec_find_decoder((AVCodecID)sub)))
		return E_INVALIDARG;


	if(NULL != pMtOut)
	{
        int width_in = 0,height_in = 0;
        int64_t duration_in = 0;

        JIF(pMtIn->GetVideoInfo(NULL,&width_in,&height_in,NULL,NULL,&duration_in));

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

