#include "FFmpegAudioEncoder.h"

CFFmpegAudioEncoder::CFFmpegAudioEncoder()
:m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
,m_ctxCodec(NULL)
{
}

bool CFFmpegAudioEncoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,(IFilter*)this),false);
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
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFFmpegAudioEncoder::GetType()
{
    return FT_Transform;
}

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
    return m_spPinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioEncoder::GetInputPin(uint32_t index)
{
    return 0 == index ? m_spPinIn : NULL;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioEncoder::GetOutputPinCount()
{
    return m_spPinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioEncoder::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_spPinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioEncoder::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioEncoder::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFFmpegAudioEncoder::Notify(uint32_t cmd)
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

STDMETHODIMP_(uint32_t) CFFmpegAudioEncoder::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFFmpegAudioEncoder::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFFmpegAudioEncoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegAudioEncoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegAudioEncoder::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    HRESULT hr = S_OK;

    if(NULL != pMT)
    {
		JIF(FilterQuery(NULL,NULL,pMT,m_spPinOut->GetMediaType()));
        JIF(pMT->GetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,
            &m_ctxCodec->channels,
            &m_ctxCodec->sample_rate));

        m_ctxCodec->time_base.num = 1;
        m_ctxCodec->time_base.den = m_ctxCodec->sample_rate;
    }
    return hr;
}

STDMETHODIMP CFFmpegAudioEncoder::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        JIF(FilterQuery(NULL,NULL,m_spPinIn == NULL ? NULL : m_spPinIn->GetMediaType(),pMT));

		AVCodec* pCodec;
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

        if(m_spPinIn == NULL)
        {
            dom_ptr<IMediaType> spMT;
            JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMT->SetSub(MST_PCM));
            JIF(spMT->SetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
                &m_ctxCodec->channel_layout,
                &m_ctxCodec->channels,
                &m_ctxCodec->sample_rate,
                &m_ctxCodec->frame_size));
            JIF(m_spPinIn->SetMediaType(spMT));
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

STDMETHODIMP CFFmpegAudioEncoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Write(pFrame);
}

HRESULT CFFmpegAudioEncoder::Open()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
    {
        IMediaType* pMT;
        JCHK(pMT = m_spPinOut->GetMediaType(),E_FAIL);
        m_ctxCodec->bit_rate = 64000;
        bool isGlobalHeader = true;
        pMT->GetStreamInfo(NULL,NULL,&m_ctxCodec->bit_rate,NULL,NULL,NULL);
        m_ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        m_ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;

        char err[AV_ERROR_MAX_STRING_SIZE] = {0};
        m_ctxCodec->thread_count = 0;
        JCHK2(0 == (hr = avcodec_open2(m_ctxCodec,avcodec_find_encoder(m_ctxCodec->codec_id),NULL)),E_FAIL,
            "Open media[%s] encoder fail msg:%s",pMT->GetSubLongName(),
            av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

        dom_ptr<IProfile> spProfile(this);
        GetOption(m_ctxCodec->priv_data,spProfile);

        JIF(pMT->SetAudioInfo((AudioMediaType*)&m_ctxCodec->sample_fmt,
            &m_ctxCodec->channel_layout,&m_ctxCodec->channels,
            &m_ctxCodec->sample_rate,&m_ctxCodec->frame_size));

        JIF(pMT->SetStreamInfo(NULL,NULL,&m_ctxCodec->bit_rate,&isGlobalHeader,m_ctxCodec->extradata,&m_ctxCodec->extradata_size));

        m_isOpen = true;
    }
	return hr;
}

HRESULT CFFmpegAudioEncoder::Close()
{
    if(true == m_isOpen)
    {
        avcodec_close(m_ctxCodec);
        m_isOpen = false;
    }
    return S_OK;
}

HRESULT CFFmpegAudioEncoder::Write(IMediaFrame* pFrame)
{
	HRESULT hr = S_OK;
	AVFrame frame;
	AVFrame* pFrameIn = NULL;
    IMediaType* pMtIn;
    JCHK(NULL != (pMtIn = m_spPinIn->GetMediaType()),E_FAIL);
	if(NULL != pFrame)
	{
		if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
		{
			avcodec_flush_buffers(m_ctxCodec);
		}
		memset(&frame,0,sizeof(frame));
		frame.extended_data = frame.data;
		JIF(FrameToAVFrame(pMtIn,&frame,pFrame,m_ctxCodec));
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
            hr = E_EOF;
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

HRESULT CFFmpegAudioEncoder::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL == pMtOut)
        return E_INVALIDARG;

	if(MMT_AUDIO != pMtOut->GetMajor())
        return E_INVALIDARG;

    HRESULT hr = S_OK;

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

