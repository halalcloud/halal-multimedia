#include "FFmpegAudioResample.h"

CFFmpegAudioResample::CFFmpegAudioResample()
:m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
,m_ctxResample(NULL)
,m_frame_size_resample(0)
,m_frame_size_out(0)
,m_sample_fmt_out(AMT_NONE)
,m_channel_out(0)
{
    //ctor
    memset(&m_info,0,sizeof(m_info));
}

bool CFFmpegAudioResample::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,(IFilter*)this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,(IFilter*)this),false);
    return true;
}
bool CFFmpegAudioResample::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegAudioResample)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFFmpegAudioResample::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CFFmpegAudioResample::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegAudioResample::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegAudioResample::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioResample::GetInputPinCount()
{
    return m_spPinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioResample::GetInputPin(uint32_t index)
{
    return 0 == index ? m_spPinIn : NULL;
}

STDMETHODIMP_(uint32_t) CFFmpegAudioResample::GetOutputPinCount()
{
    return m_spPinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioResample::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_spPinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CFFmpegAudioResample::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFFmpegAudioResample::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFFmpegAudioResample::Notify(uint32_t cmd)
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
            else if(IFilter::S_Stop == cmd)
            {
                JIF(Close());
            }
            m_status = (IFilter::Status)cmd;
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

STDMETHODIMP_(uint32_t) CFFmpegAudioResample::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFFmpegAudioResample::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFFmpegAudioResample::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegAudioResample::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegAudioResample::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    return FilterQuery(NULL,NULL,pMT,m_spPinOut->GetMediaType());
}

STDMETHODIMP CFFmpegAudioResample::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return FilterQuery(NULL,NULL,m_spPinIn->GetMediaType(),pMT);
}

STDMETHODIMP CFFmpegAudioResample::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Write(pFrame);
}

HRESULT CFFmpegAudioResample::Open()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
    {
        IMediaType* pMtIn;
        IMediaType* pMtOut;
        JCHK(NULL != (pMtIn = m_spPinIn->GetMediaType()),E_FAIL);
        JCHK(NULL != (pMtOut = m_spPinOut->GetMediaType()),E_FAIL);

        AudioMediaType sample_fmt_in;
        uint64_t channel_layout_in,channel_layout_out;
        int channel_in,sample_rate_in,sample_rate_out,frame_size_in;
        JIF(pMtIn->GetAudioInfo(&sample_fmt_in,&channel_layout_in,&channel_in,&sample_rate_in,&frame_size_in));
        JIF(pMtOut->GetAudioInfo(&m_sample_fmt_out,&channel_layout_out,&m_channel_out,&sample_rate_out,&m_frame_size_out));

        if( sample_fmt_in != m_sample_fmt_out || channel_layout_in != channel_layout_out || sample_rate_in != sample_rate_out)
        {

            JCHK(m_ctxResample = swr_alloc(),E_FAIL);

            JCHK(0 == av_opt_set_sample_fmt(m_ctxResample, "in_sample_fmt", (AVSampleFormat)sample_fmt_in, 0),E_FAIL);
            JCHK(0 == av_opt_set_int(m_ctxResample, "in_channel_layout",    channel_layout_in, 0),E_FAIL);
            JCHK(0 == av_opt_set_int(m_ctxResample, "in_sample_rate",       sample_rate_in, 0),E_FAIL);

            JCHK(0 == av_opt_set_sample_fmt(m_ctxResample, "out_sample_fmt", (AVSampleFormat)m_sample_fmt_out, 0),E_FAIL);
            JCHK(0 == av_opt_set_int(m_ctxResample, "out_channel_layout",    channel_layout_out, 0),E_FAIL);
            JCHK(0 == av_opt_set_int(m_ctxResample, "out_sample_rate",       sample_rate_out, 0),E_FAIL);

            JIF(swr_init(m_ctxResample));

            m_frame_size_resample = av_rescale_rnd(frame_size_in, sample_rate_out, sample_rate_in, AV_ROUND_DOWN);
            if(m_frame_size_resample != m_frame_size_out)
            {
                JCHK(m_spMt.Create(CLSID_CMediaType),E_FAIL);
                JIF(m_spMt->SetSub(MST_PCM));
                JIF(m_spMt->SetAudioInfo(&m_sample_fmt_out,&channel_layout_out,&m_channel_out,&sample_rate_out,&m_frame_size_resample));
                m_pMtResample = m_spMt.p;
            }
            else
                m_pMtResample = pMtOut;
        }
        memset(&m_info,0,sizeof(m_info));
        m_info.dts = MEDIA_FRAME_NONE_TIMESTAMP;
        m_info.pts = MEDIA_FRAME_NONE_TIMESTAMP;
        m_isOpen = true;
    }
	return hr;
}

HRESULT CFFmpegAudioResample::Close()
{
    if(true == m_isOpen)
    {
        if(NULL != m_ctxResample)
            swr_free(&m_ctxResample);
        m_spFrameSample = NULL;
        m_spMt = NULL;
        m_isOpen = false;
    }
    return S_OK;
}

HRESULT CFFmpegAudioResample::Write(IMediaFrame* pFrame)
{
    if(NULL == pFrame)
        return E_EOF;
	if(NULL != m_ctxResample)
	{
        HRESULT hr = S_OK;
		uint8_t *dataIn[AV_NUM_DATA_POINTERS];
		int linesizeIn[AV_NUM_DATA_POINTERS];

        uint8_t *dataOut[AV_NUM_DATA_POINTERS];
		int linesizeOut[AV_NUM_DATA_POINTERS];

        IMediaType* pMtIn;
        JCHK(NULL != (pMtIn = m_spPinIn->GetMediaType()),E_FAIL);

        dom_ptr<IMediaFrame> spFrame;
        JIF(m_spPinOut->AllocFrame(&spFrame));
        spFrame->info = pFrame->info;
        JIF(m_pMtResample->FrameAlloc(spFrame));
        JIF(pMtIn->FrameToArray(dataIn,linesizeIn,pFrame));
        JIF(m_pMtResample->FrameToArray(dataOut,linesizeOut,spFrame));
		char err[AV_ERROR_MAX_STRING_SIZE] = {0};
		JCHK1(0 <= (hr = swr_convert(m_ctxResample,dataOut,spFrame->info.samples,(const uint8_t **)dataIn,pFrame->info.samples)),
			S_FALSE,"audio resample frame fail:%s",av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
        return Sample(spFrame);
    }
    else
        return Sample(pFrame);
}

HRESULT CFFmpegAudioResample::Sample(IMediaFrame* pFrame)
{
    if(pFrame->info.samples != m_frame_size_out)
    {
        HRESULT hr = S_OK;

		uint8_t *dataIn[AV_NUM_DATA_POINTERS];
		int linesizeIn[AV_NUM_DATA_POINTERS];

		uint8_t *dataOut[AV_NUM_DATA_POINTERS];
		int linesizeOut[AV_NUM_DATA_POINTERS];

        IMediaType* pMtOut;
        JCHK(NULL != (pMtOut = m_spPinOut->GetMediaType()),E_FAIL);

        JIF(pMtOut->FrameToArray(dataIn,linesizeIn,pFrame));

        unsigned int offsetIn = 0,deltaIn,deltaOut;

        if(MEDIA_FRAME_NONE_TIMESTAMP == pFrame->info.pts)
            pFrame->info.pts = MEDIA_FRAME_NONE_TIMESTAMP == m_info.pts ? 0 : m_info.pts;
        if(pFrame->info.pts != pFrame->info.dts)
            pFrame->info.dts = pFrame->info.pts;

        if(MEDIA_FRAME_NONE_TIMESTAMP != m_info.pts)
        {
            int64_t delta = pFrame->info.pts - m_info.pts;
            if(delta > m_info.duration || delta < -m_info.duration)
            {
                LOG(1,"audio sample frame PTS:%ldms timestamp exception prev frame PTS:%ldms delta:%ld",
                    int(pFrame->info.pts/10000),int(m_info.pts/10000),delta);
                if(0 > delta)
                    delta = -m_info.duration/2;
                m_info.pts += delta;
                m_info.dts = m_info.pts;
            }
        }
        else
        {
            m_info = pFrame->info;
            m_info.samples = 0;
        }

        while((deltaIn = (pFrame->info.samples - offsetIn)) >= (deltaOut = ((unsigned int)m_frame_size_out - m_info.samples)))
        {
            if(m_spFrameSample == NULL)
            {
                JIF(m_spPinOut->AllocFrame(&m_spFrameSample));
                JIF(pMtOut->FrameAlloc(m_spFrameSample));
                m_info.duration = m_spFrameSample->info.duration;
            }
            JIF(pMtOut->FrameToArray(dataOut,linesizeOut,m_spFrameSample));
			JCHK(0 <= av_samples_copy(dataOut,
				dataIn,
				m_info.samples,
				offsetIn,
				deltaOut,
				m_channel_out,
				(AVSampleFormat)m_sample_fmt_out),S_FALSE);
            m_spFrameSample->info = m_info;
            m_spFrameSample->info.samples = m_frame_size_out;
            hr = m_spPinOut->Write(m_spFrameSample);
            m_spFrameSample = NULL;
            if(0 != m_info.tag)
                m_info.tag = 0;
            if(NULL != m_info.pExt)
                m_info.pExt = NULL;
            m_info.pts += m_info.duration;
            m_info.dts += m_info.duration;
            offsetIn += deltaOut;
            m_info.samples = 0;
        }
        if(deltaIn > 0)
        {
            if(m_spFrameSample == NULL)
            {
                JIF(m_spPinOut->AllocFrame(&m_spFrameSample));
                JIF(pMtOut->FrameAlloc(m_spFrameSample));
                m_info.duration = m_spFrameSample->info.duration;
            }
            JIF(pMtOut->FrameToArray(dataOut,linesizeOut,m_spFrameSample));
			JCHK(0 <= av_samples_copy(dataOut,
				dataIn,
				m_info.samples,
				offsetIn,
				deltaIn,
				m_channel_out,
				(AVSampleFormat)m_sample_fmt_out),S_FALSE);
            m_info.samples += deltaIn;
        }
        return hr;
    }
    else
        return m_spPinOut->Write(pFrame);
}

HRESULT CFFmpegAudioResample::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr = S_OK;

    if(NULL != pMtIn)
    {
        if(MST_PCM != pMtIn->GetSub())
            return E_INVALIDARG;
    }
    if(NULL != pMtOut)
    {
        if(MST_PCM != pMtOut->GetSub())
            return E_INVALIDARG;
    }
    if(NULL != pMtIn && NULL != pMtOut)
    {
        AVSampleFormat sample_fmt_in,sample_fmt_out;
        int channels_in,channels_out,sample_rate_in,sample_rate_out,frame_size_in,frame_size_out;
        uint64_t channel_layout_in,channel_layout_out;
        JIF(pMtIn->GetAudioInfo((AudioMediaType*)&sample_fmt_in,&channel_layout_in,&channels_in,&sample_rate_in,&frame_size_in));
        JIF(pMtOut->GetAudioInfo((AudioMediaType*)&sample_fmt_out,&channel_layout_out,&channels_out,&sample_rate_out,&frame_size_out));
    }
	return hr;
}
