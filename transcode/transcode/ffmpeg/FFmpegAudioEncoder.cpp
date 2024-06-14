#include "FFmpegAudioEncoder.h"

CFFmpegAudioEncoder::CFFmpegAudioEncoder()
:m_pFG(NULL)
,m_isOpen(false)
,m_pTag(NULL)
,m_pMtIn(NULL)
,m_pMtOut(NULL)
,m_ctxCodec(NULL)
{
}

bool CFFmpegAudioEncoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,dynamic_cast<IFilter*>(this)),false);
    return true;
}

bool CFFmpegAudioEncoder::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegAudioEncoder)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CFFmpegAudioEncoder::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegAudioEncoder::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegAudioEncoder::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioEncoder::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioEncoder::GetInputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinIn;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioEncoder::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioEncoder::GetOutputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinOut;
}

STDMETHODIMP CFFmpegAudioEncoder::Open()
{
    HRESULT hr = S_OK;
    JCHK(NULL != m_pMtIn,E_FAIL);
    JCHK(NULL != m_pMtOut,E_FAIL);
    if(false == m_isOpen)
    {
        m_ctxCodec->bit_rate = 64000;
        bool isGlobalHeader = true;
        m_pMtOut->GetStreamInfo(NULL,NULL,&m_ctxCodec->bit_rate,&isGlobalHeader,NULL,NULL);
        if(true == isGlobalHeader)
        {
            m_ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            m_ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
        }
        else
        {
            //m_ctxCodec->flags &= ~CODEC_FLAG_GLOBAL_HEADER;
            //m_ctxCodec->flags2 |= AV_CODEC_FLAG2_LOCAL_HEADER;
            m_ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            m_ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
        }

        char err[AV_ERROR_MAX_STRING_SIZE] = {0};
        m_ctxCodec->thread_count = 0;
        JCHK2(0 == (hr = avcodec_open2(m_ctxCodec,avcodec_find_encoder(m_ctxCodec->codec_id),NULL)),E_FAIL,
            "Open media[%s] encoder fail msg:%s",m_pMtOut->GetSubLongName(),
            av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

        dom_ptr<IProfile> spProfile(this);
        GetOption(m_ctxCodec->priv_data,spProfile);

        JIF(m_pMtOut->SetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,&m_ctxCodec->channels,
            &m_ctxCodec->sample_rate,&m_ctxCodec->frame_size));

        JIF(m_pMtOut->SetStreamInfo(NULL,NULL,&m_ctxCodec->bit_rate,&isGlobalHeader,m_ctxCodec->extradata,&m_ctxCodec->extradata_size));
        m_isOpen = true;
    }
	return hr;
}

STDMETHODIMP CFFmpegAudioEncoder::Close()
{
    if(true == m_isOpen)
    {
        avcodec_close(m_ctxCodec);
        m_isOpen = false;
    }
    return S_OK;
}

STDMETHODIMP CFFmpegAudioEncoder::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CFFmpegAudioEncoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegAudioEncoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegAudioEncoder::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    if(NULL == m_pMtOut || NULL == m_ctxCodec)
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

STDMETHODIMP CFFmpegAudioEncoder::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CFFmpegAudioEncoder::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;

    if(NULL != pMT)
    {
        JIF(CheckMediaType(pMT,m_pMtOut));
        JIF(pMT->GetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,
            &m_ctxCodec->channels,
            &m_ctxCodec->sample_rate));

        m_ctxCodec->time_base.num = 1;
        m_ctxCodec->time_base.den = m_ctxCodec->sample_rate;
    }
    m_pMtIn = pMT;
    return hr;
}

STDMETHODIMP CFFmpegAudioEncoder::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
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

        JIF(pMT->GetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,&m_ctxCodec->channels,
            &m_ctxCodec->sample_rate,NULL));

        if(NULL != pCodec->sample_fmts)
        {
            int i=0;
            while(AV_SAMPLE_FMT_NONE != pCodec->sample_fmts[i] && m_ctxCodec->sample_fmt != pCodec->sample_fmts[i]){++i;}
            if(AV_SAMPLE_FMT_NONE == pCodec->sample_fmts[i])
            {
                LOG(1,"audio encoder not support sample format:%d change to format:%d",m_ctxCodec->sample_fmt,pCodec->sample_fmts[0]);
                m_ctxCodec->sample_fmt = pCodec->sample_fmts[0];
            }
        }

        if(NULL != pCodec->channel_layouts)
        {
            int i=0;
            while(0 != pCodec->channel_layouts[i] && m_ctxCodec->channel_layout != pCodec->channel_layouts[i]){++i;}
            if(0 == pCodec->channel_layouts[i])
            {
                LOG(1,"audio encoder not support channel layout:%d change to channel layout:%d",m_ctxCodec->channel_layout,pCodec->channel_layouts[0]);
                m_ctxCodec->channel_layout = pCodec->channel_layouts[0];
            }
        }

        if(NULL != pCodec->supported_samplerates)
        {
            int tmp = m_ctxCodec->sample_rate;
            GetAudioSampleRate(pCodec->supported_samplerates,tmp);
            if(tmp != m_ctxCodec->sample_rate)
            {
                LOG(1,"audio encoder not support sample rate:%d change to sample rate:%d",m_ctxCodec->sample_rate,tmp);
                m_ctxCodec->sample_rate = tmp;
            }
        }

        m_ctxCodec->time_base.num = 1;
        m_ctxCodec->time_base.den = m_ctxCodec->sample_rate;

        pCodec->init(m_ctxCodec);
        JCHK(0 < m_ctxCodec->frame_size,E_FAIL);
        m_ctxCodec->block_align = AUDIO_ALIGN;
        JIF(pMT->SetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,&m_ctxCodec->channels,
            &m_ctxCodec->sample_rate,&m_ctxCodec->frame_size));
        JIF(pMT->SetStreamInfo());
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

STDMETHODIMP CFFmpegAudioEncoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
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

STDMETHODIMP CFFmpegAudioEncoder::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

HRESULT CFFmpegAudioEncoder::Write(IMediaFrame* pFrame)
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
		memset(&frame,0,sizeof(frame));
		frame.extended_data = frame.data;
		JIF(FrameToAVFrame(m_pMtIn,&frame,pFrame,m_ctxCodec));
		pFrameIn = &frame;
	}
    AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	int isOutput = 0;
	hr = avcodec_encode_audio2(m_ctxCodec,&pkt,pFrameIn,&isOutput);
	if(0 != isOutput)
	{
        dom_ptr<IMediaFrame> spFrame;
        JIF(m_spPinOut->AllocFrame(&spFrame));
		JCHK(S_OK == AVPacketToFrame(spFrame,pkt,m_ctxCodec->time_base,m_ctxCodec->frame_size),S_FALSE);
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
            JCHK2(0 == hr,S_FALSE,"ffmpeg audio encode frame[DTS:%I64d] fail,msg:%s"
                ,pFrame->info.dts,av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr))
        }
        else
            hr = S_OK;
	}
	return hr;
}

HRESULT CFFmpegAudioEncoder::CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr = S_OK;

    if(NULL == pMtOut)
        return E_INVALIDARG;

	if(MMT_AUDIO != pMtOut->GetMajor())
        return E_INVALIDARG;

    AVCodec* pCodec;
	MediaSubType sub = pMtOut->GetSub();
	if(sub == MST_PCM || NULL == (pCodec = avcodec_find_encoder((AVCodecID)sub)))
		return E_INVALIDARG;

    if(NULL != pMtIn)
    {
        AVSampleFormat sample_fmt_out = AV_SAMPLE_FMT_NONE;
        int channels_out = 0,sample_rate_out = 0,frame_size_out = 0;
        uint64_t channel_layout_out = 0;

        JIF(pMtOut->GetAudioInfo((AudioMediaType*)&sample_fmt_out,&channel_layout_out,
            &channels_out,&sample_rate_out,&frame_size_out));

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

        if(NULL != pCodec->supported_samplerates)
        {
            int i=0;
            while(0 != pCodec->supported_samplerates[i] && sample_rate_out != pCodec->supported_samplerates[i]){++i;}
            if(0 == pCodec->supported_samplerates[i])
                return E_INVALIDARG;
        }

        if(MST_PCM != pMtIn->GetSub())
            return E_INVALIDARG;

        AVSampleFormat sample_fmt_in = AV_SAMPLE_FMT_NONE;
        int channels_in = 0,sample_rate_in = 0,frame_size_in = 0;
        uint64_t channel_layout_in = 0;
        JIF(pMtIn->GetAudioInfo((AudioMediaType*)&sample_fmt_in,&channel_layout_in,
            &channels_in,&sample_rate_in,&frame_size_in));

        if(sample_fmt_out != sample_fmt_in)
            return E_INVALIDARG;

        if(channel_layout_out != channel_layout_in)
            return E_INVALIDARG;

        if(sample_rate_out != sample_rate_in)
            return E_INVALIDARG;

        if(frame_size_out != frame_size_in)
            return E_INVALIDARG;
    }
	return hr;
}

