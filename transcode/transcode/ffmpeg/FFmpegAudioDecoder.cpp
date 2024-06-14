#include "FFmpegAudioDecoder.h"

CFFmpegAudioDecoder::CFFmpegAudioDecoder()
:m_pFG(NULL)
,m_isOpen(false)
,m_pTag(NULL)
,m_pMtIn(NULL)
,m_pMtOut(NULL)
,m_ctxCodec(NULL)
,m_pFrame(NULL)
{
	av_init_packet(&m_pkt);
}

bool CFFmpegAudioDecoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
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
DOM_QUERY_IMPLEMENT_END

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
    return 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioDecoder::GetInputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinIn;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioDecoder::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioDecoder::GetOutputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinOut;
}

STDMETHODIMP CFFmpegAudioDecoder::Open()
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
        m_isOpen = true;
    }
	return hr;
}

STDMETHODIMP CFFmpegAudioDecoder::Close()
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

STDMETHODIMP CFFmpegAudioDecoder::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CFFmpegAudioDecoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegAudioDecoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegAudioDecoder::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CFFmpegAudioDecoder::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    if(NULL == m_pMtIn || NULL == m_ctxCodec)
        return E_FAIL;
    else
    {
        HRESULT hr;
        JIF(pMT->Clear());
        JIF(pMT->SetSub(MST_PCM));
        JIF(pMT->SetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,
            &m_ctxCodec->channels,
            &m_ctxCodec->sample_rate,
            &m_ctxCodec->frame_size));
        return hr;
    }
}

STDMETHODIMP CFFmpegAudioDecoder::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
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

STDMETHODIMP CFFmpegAudioDecoder::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
		JIF(CheckMediaType(m_pMtIn,pMT));

        JIF(pMT->GetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,&m_ctxCodec->channels,
            &m_ctxCodec->sample_rate));

        m_ctxCodec->time_base = FRAME_TIMEBASE;
    }
    m_pMtOut = pMT;
    return hr;
}

STDMETHODIMP CFFmpegAudioDecoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
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

STDMETHODIMP CFFmpegAudioDecoder::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

HRESULT CFFmpegAudioDecoder::Write(IMediaFrame* pFrame)
{
	HRESULT hr = S_FALSE;
	AVPacket pkt;
	av_init_packet(&pkt);

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
			m_spPinOut->NewSegment();
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
            JIF(AVFrameToFrame(m_pMtOut,spFrame,m_pFrame));
            //printf("audio decode out frame PTS:%ld\n",spFrame->info.pts/10000);
            JIF(m_spPinOut->Write(spFrame));
        }
        else
        {
            if(NULL == m_pkt.data)
                return S_STREAM_EOF;
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

HRESULT CFFmpegAudioDecoder::CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr;
	if(NULL == pMtIn)
		return E_INVALIDARG;

	if(MMT_AUDIO != pMtIn->GetMajor())
        return E_INVALIDARG;

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

