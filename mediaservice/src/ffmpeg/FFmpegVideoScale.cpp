#include "FFmpegVideoScale.h"

CFFmpegVideoScale::CFFmpegVideoScale()
:m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
,m_ctxSws(NULL)
,m_height_in(0)
,m_height_out(0)
,m_duration_in(0)
,m_duration_out(0)
{
    //ctor
}

bool CFFmpegVideoScale::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,(IFilter*)this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,(IFilter*)this),false);
    return true;
}

bool CFFmpegVideoScale::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegVideoScale)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFFmpegVideoScale::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CFFmpegVideoScale::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegVideoScale::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegVideoScale::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoScale::GetInputPinCount()
{
    return m_spPinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegVideoScale::GetInputPin(uint32_t index)
{
    return 0 == index ? m_spPinIn : NULL;
}

STDMETHODIMP_(uint32_t) CFFmpegVideoScale::GetOutputPinCount()
{
    return m_spPinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegVideoScale::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_spPinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CFFmpegVideoScale::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFFmpegVideoScale::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFFmpegVideoScale::Notify(uint32_t cmd)
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

STDMETHODIMP_(uint32_t) CFFmpegVideoScale::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFFmpegVideoScale::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFFmpegVideoScale::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegVideoScale::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegVideoScale::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    return FilterQuery(NULL,NULL,pMT,m_spPinOut->GetMediaType());
}

STDMETHODIMP CFFmpegVideoScale::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return FilterQuery(NULL,NULL,m_spPinIn->GetMediaType(),pMT);
}

STDMETHODIMP CFFmpegVideoScale::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Write(pFrame);
}

HRESULT CFFmpegVideoScale::Open()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
    {
        IMediaType* pMtIn;
        IMediaType* pMtOut;
        JCHK(NULL != (pMtIn = m_spPinIn->GetMediaType()),E_FAIL);
        JCHK(NULL != (pMtOut = m_spPinOut->GetMediaType()),E_FAIL);

        AVPixelFormat pix_fmt_in,pix_fmt_out;
        int width_in,width_out;
        JIF(pMtIn->GetVideoInfo((VideoMediaType*)&pix_fmt_in,&width_in,&m_height_in,NULL,NULL,&m_duration_in));
        JIF(pMtOut->GetVideoInfo((VideoMediaType*)&pix_fmt_out,&width_out,&m_height_out,NULL,NULL,&m_duration_out));
        if(pix_fmt_in != pix_fmt_out || width_in != width_out || m_height_in != m_height_out)
        {
            JCHK(m_ctxSws = sws_getContext(width_in,m_height_in,pix_fmt_in,width_out,m_height_out,pix_fmt_out,SWS_BICUBIC, NULL, NULL, NULL),E_FAIL);
        }
        m_isOpen = true;
    }
	return hr;
}

HRESULT CFFmpegVideoScale::Close()
{
    if(true == m_isOpen)
    {
        if(NULL != m_ctxSws)
        {
            sws_freeContext(m_ctxSws);
            m_ctxSws = NULL;
        }
        m_spFrameSample = NULL;
        m_isOpen = false;
    }
    return S_OK;
}

HRESULT CFFmpegVideoScale::Write(IMediaFrame* pFrame)
{
    if(NULL == pFrame)
        return E_EOF;
    if(m_duration_out >= m_duration_in)
        return Sample(pFrame,true);
    else
        return Scale(pFrame,true);
}

HRESULT CFFmpegVideoScale::Scale(IMediaFrame* pFrame,bool isSample)
{
    dom_ptr<IMediaFrame> spFrame;
    if(NULL != m_ctxSws)
    {
        HRESULT hr;

		uint8_t *dataIn[AV_NUM_DATA_POINTERS];
		int linesizeIn[AV_NUM_DATA_POINTERS];

        uint8_t *dataOut[AV_NUM_DATA_POINTERS];
		int linesizeOut[AV_NUM_DATA_POINTERS];

        IMediaType* pMtIn;
        IMediaType* pMtOut;
        JCHK(pMtIn = m_spPinIn->GetMediaType(),E_FAIL);
        JCHK(pMtOut = m_spPinOut->GetMediaType(),E_FAIL);
        JIF(m_spPinOut->AllocFrame(&spFrame));
        JIF(pMtOut->FrameAlloc(spFrame));
        int stride = spFrame->info.stride;
        spFrame->info = pFrame->info;
        spFrame->info.stride = stride;

        JIF(pMtIn->FrameToArray(dataIn,linesizeIn,pFrame));
        JIF(pMtOut->FrameToArray(dataOut,linesizeOut,spFrame));

		char err[AV_ERROR_MAX_STRING_SIZE] = {0};
		//_tprintf(_T("sws_scale:%I64d in\n"),pFrameIn->info.pts);
		JCHK2(m_height_out == sws_scale(m_ctxSws,dataIn,linesizeIn,0,m_height_in,dataOut,linesizeOut),
            S_FALSE,"ffmpeg video scale frame[PTS:%l] fail,msg:%s",pFrame->info.pts,
			av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
        pFrame = spFrame.p;
    }
    return true == isSample ? Sample(pFrame,false) : m_spPinOut->Write(pFrame);
}

HRESULT CFFmpegVideoScale::Sample(IMediaFrame* pFrame,bool isScale)
{
    if(m_duration_in != m_duration_out)
    {
        HRESULT hr = S_OK;
        if(m_spFrameSample == NULL)
        {
            m_spFrameSample = pFrame;
            m_spFrameSample->info.duration = m_duration_out;
            return SampleOut(m_spFrameSample,isScale);
        }
        else
        {
            int64_t delta = pFrame->info.pts - m_spFrameSample->info.pts;
            if(delta >= m_duration_out)
            {
                do
                {
                    dom_ptr<IMediaFrame> spFrame;
                    JIF(m_spPinOut->AllocFrame(&spFrame));
                    JIF(spFrame->CopyFrom(m_spFrameSample));
                    spFrame->info.pts += m_duration_out;
                    spFrame->info.dts += m_duration_out;
                    spFrame->info.flag = 0;
                    JIF(SampleOut(spFrame,isScale));
                    delta -= m_duration_out;
                    m_spFrameSample = spFrame;
                }while(delta >= m_duration_out);
            }
            JIF(m_spFrameSample->CopyFrom(pFrame,MEDIAFRAME_COPY_DATA));
            return hr;
        }
    }
    else
        return SampleOut(pFrame,isScale);
}

HRESULT CFFmpegVideoScale::SampleOut(IMediaFrame* pFrame,bool isScale)
{
    return true == isScale ? Scale(pFrame,false) : m_spPinOut->Write(pFrame);
}

HRESULT CFFmpegVideoScale::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr = S_OK;
    AVPixelFormat pix_fmt_in,pix_fmt_out;
    int width_in,width_out,height_in,height_out;
    int64_t duration_in,duration_out;
    if(NULL != pMtIn)
    {
        if(MST_RAWVIDEO != pMtIn->GetSub())
            return E_INVALIDARG;
        JIF(pMtIn->GetVideoInfo((VideoMediaType*)&pix_fmt_in,&width_in,&height_in,NULL,NULL,&duration_in));
    }
    if(NULL != pMtOut)
    {
        if(MST_RAWVIDEO != pMtOut->GetSub())
            return E_INVALIDARG;
        JIF(pMtOut->GetVideoInfo((VideoMediaType*)&pix_fmt_out,&width_out,&height_out,NULL,NULL,&duration_out));
    }
	return hr;

}
