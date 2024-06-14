#include "FFmpegAudioDecoder.h"

CFFmpegAudioDecoder::CFFmpegAudioDecoder()
:m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
,m_ctxCodec(NULL)
,m_pFrame(NULL)
{
	av_init_packet(&m_pkt);
}

bool CFFmpegAudioDecoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,(IFilter*)this),false);
    return true;
}

bool CFFmpegAudioDecoder::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegAudioDecoder)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFFmpegAudioDecoder::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CFFmpegAudioDecoder::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegAudioDecoder::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegAudioDecoder::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioDecoder::GetInputPinCount()
{
    return m_spPinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioDecoder::GetInputPin(uint32_t index)
{
    return 0 == index ? m_spPinIn : NULL;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioDecoder::GetOutputPinCount()
{
    return m_spPinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioDecoder::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_spPinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioDecoder::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioDecoder::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFFmpegAudioDecoder::Notify(uint32_t cmd)
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

STDMETHODIMP_(uint32_t) CFFmpegAudioDecoder::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFFmpegAudioDecoder::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFFmpegAudioDecoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegAudioDecoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegAudioDecoder::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
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
        bool global_header = false;
        pMT->GetStreamInfo(NULL,NULL,NULL,&global_header,&m_ctxCodec->extradata,&m_ctxCodec->extradata_size);

        if(true == global_header)
        {
            m_ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            m_ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
        }
        else
        {
            m_ctxCodec->flags &= ~CODEC_FLAG_GLOBAL_HEADER;
            m_ctxCodec->flags2 |= AV_CODEC_FLAG2_LOCAL_HEADER;
        }

        JIF(pMT->GetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,&m_ctxCodec->channels,
            &m_ctxCodec->sample_rate,&m_ctxCodec->frame_size));

        m_ctxCodec->time_base = FRAME_TIMEBASE;

        if(AV_CODEC_ID_WMAPRO == id)
            m_ctxCodec->frame_size = 2048;

        if(m_spPinOut == NULL)
        {
            dom_ptr<IMediaType> spMT;
            JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMT->SetSub(MST_PCM));
            JIF(spMT->SetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
                &m_ctxCodec->channel_layout,
                &m_ctxCodec->channels,
                &m_ctxCodec->sample_rate,
                &m_ctxCodec->frame_size));
            JIF(m_spPinOut->SetMediaType(spMT));
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

STDMETHODIMP CFFmpegAudioDecoder::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
		JIF(FilterQuery(NULL,NULL,m_spPinIn->GetMediaType(),pMT));

        JIF(pMT->GetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,&m_ctxCodec->channels,
            &m_ctxCodec->sample_rate));

        m_ctxCodec->time_base = FRAME_TIMEBASE;
    }
    return hr;
}

STDMETHODIMP CFFmpegAudioDecoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Write(pFrame);
}

HRESULT CFFmpegAudioDecoder::Open()
{
    if(false == m_isOpen)
    {
        IMediaType* pMtIn;
        JCHK(NULL != (pMtIn = m_spPinIn->GetMediaType()),E_FAIL);
        m_ctxCodec->thread_count = 0;
        JCHK1(0 == avcodec_open2(m_ctxCodec,avcodec_find_decoder(m_ctxCodec->codec_id),NULL),E_FAIL,"Open media[%s] decoder fail",pMtIn->GetSubLongName());
        JCHK(m_pFrame = av_frame_alloc(),E_FAIL);
        av_init_packet(&m_pkt);
        m_isOpen = true;
    }
    return S_OK;
}

HRESULT CFFmpegAudioDecoder::Close()
{
    if(true == m_isOpen)
    {
        av_frame_free(&m_pFrame);
        avcodec_close(m_ctxCodec);
        m_isOpen = false;
    }
    return S_OK;
}

HRESULT CFFmpegAudioDecoder::Write(IMediaFrame* pFrame)
{
	HRESULT hr = S_FALSE;
	AVPacket pkt;
	av_init_packet(&pkt);

    IMediaType* pMtOut;
    JCHK(NULL != (pMtOut = m_spPinOut->GetMediaType()),E_FAIL);

	if(pFrame == NULL)
	{
		m_pkt.buf = NULL;
		m_pkt.data = NULL;
		m_pkt.size = 0;
	}
	else
	{
		if(0 != (pkt.flags & MEDIA_FRAME_FLAG_NEWSEGMENT))
		{
			avcodec_flush_buffers(m_ctxCodec);
		}
		JCHK(S_OK == FrameToAVPacket(&m_pkt,pFrame),S_FALSE);
	}

	int isOutput = 0;
	int64_t pts = MEDIA_FRAME_NONE_TIMESTAMP;
    do
    {
        int cb = avcodec_decode_audio4(m_ctxCodec,m_pFrame, &isOutput,&m_pkt);
        if(0 != isOutput)
        {
            if(m_pFrame->pkt_pts <= pts)
                m_pFrame->pkt_pts = pts;

            if(MEDIA_FRAME_NONE_TIMESTAMP == pts)
                pts = m_pFrame->pkt_pts;
            pts += m_pFrame->nb_samples;
            if(m_pFrame->pkt_dts != m_pFrame->pkt_pts)
                m_pFrame->pkt_dts = m_pFrame->pkt_pts;
            dom_ptr<IMediaFrame> spFrame;
            JIF(m_spPinOut->AllocFrame(&spFrame));
            JIF(AVFrameToFrame(pMtOut,spFrame,m_pFrame));
            //printf("audio decode out frame PTS:%ld\n",spFrame->info.pts/10000);
            JIF(m_spPinOut->Write(spFrame));
        }
        else
        {
            if(NULL == m_pkt.data)
                return E_EOF;
            else if(0 > cb)
            {
                char err[AV_ERROR_MAX_STRING_SIZE] = {0};
                JCHK2(0 <= cb,S_FALSE,"ffmpeg decode frame[DTS:%I64d] fail,msg:%s"
                    ,pFrame->info.dts/10000,av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr))
            }
        }
        m_pkt.size -= cb;
        m_pkt.data += cb;
    }while(0 < m_pkt.size);
    return hr;
}

HRESULT CFFmpegAudioDecoder::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
	if(NULL == pMtIn)
		return E_INVALIDARG;

	if(MMT_AUDIO != pMtIn->GetMajor())
        return E_INVALIDARG;

    HRESULT hr;

	AVCodec* pCodec;
	MediaSubType sub = pMtIn->GetSub();
	if(sub == MST_PCM || NULL == (pCodec = avcodec_find_decoder((AVCodecID)sub)))
		return E_INVALIDARG;

    AVSampleFormat sample_fmt_in;
    int channels_in = 0,sample_rate_in = 0,frame_size_in = 0;
    uint64_t channel_layout_in = 0;

    JIF(pMtIn->GetAudioInfo((AudioMediaType*)&sample_fmt_in,&channel_layout_in,&channels_in,&sample_rate_in,&frame_size_in));

    if(NULL != pMtOut)
    {
        if(MST_PCM != pMtOut->GetSub())
            return E_INVALIDARG;

        AVSampleFormat sample_fmt_out = AV_SAMPLE_FMT_NONE;
        int channels_out = 0,sample_rate_out = 0,frame_size_out = 0;
        uint64_t channel_layout_out = 0;
        JIF(pMtOut->GetAudioInfo((AudioMediaType*)&sample_fmt_out,&channel_layout_out,&channels_out,&sample_rate_out,&frame_size_out));

        if(NULL != pCodec->sample_fmts)
        {
            int i=0;
            while(AV_SAMPLE_FMT_NONE != pCodec->sample_fmts[i] && sample_fmt_out != pCodec->sample_fmts[i]){++i;}
            if(AV_SAMPLE_FMT_NONE == pCodec->sample_fmts[i])
                return E_INVALIDARG;
        }

        if(NULL != pCodec->channel_layouts)
        {
            int i=0;
            while(0 != pCodec->channel_layouts[i] && channel_layout_out != pCodec->channel_layouts[i]){++i;}
            if(0 == pCodec->channel_layouts[i])
                return E_INVALIDARG;
        }
        else if(channel_layout_out != channel_layout_in)
            return E_INVALIDARG;

        if(NULL != pCodec->supported_samplerates)
        {
            int i=0;
            while(0 != pCodec->supported_samplerates[i] && sample_rate_out != pCodec->supported_samplerates[i]){++i;}
            if(0 == pCodec->supported_samplerates[i])
                return E_INVALIDARG;
        }
        else if(sample_rate_out != sample_rate_in)
            return E_INVALIDARG;

        if(frame_size_out != frame_size_in)
        {
            if(MST_WMAPRO != pMtIn->GetSub() && 2048 != frame_size_out)
                return E_INVALIDARG;
        }
    }
	return hr;
}


