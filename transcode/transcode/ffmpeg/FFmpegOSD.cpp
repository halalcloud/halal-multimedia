#include "FFmpegOSD.h"

CFFmpegOSD::CFFmpegOSD()
:m_pFG(NULL)
,m_isOpen(false)
,m_pTag(NULL)
,m_pMtIn(NULL)
,m_pMtOut(NULL)
,m_graph(NULL)
,m_ctx_input(NULL)
,m_ctx_output(NULL)
,m_ctxSws(NULL)
,m_vmt(VMT_NONE)
,m_width(0)
,m_height(0)
{
    //ctor
}

bool CFFmpegOSD::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,dynamic_cast<IFilter*>(this)),false);
    return true;
}

bool CFFmpegOSD::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegOSD)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CFFmpegOSD::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegOSD::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegOSD::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegOSD::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CFFmpegOSD::GetInputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinIn;
}

STDMETHODIMP_(uint32_t) CFFmpegOSD::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CFFmpegOSD::GetOutputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinOut;
}

STDMETHODIMP CFFmpegOSD::Open()
{
    HRESULT hr = S_OK;
    JCHK(NULL != m_pMtIn,E_FAIL);
    JCHK(NULL != m_pMtOut,E_FAIL);
    if(false == m_isOpen)
    {
        dom_ptr<IProfile> spProfile;
        JCHK(spProfile.p = static_cast<IProfile*>(Query(IID(IProfile))),E_FAIL);

        IProfile::val* pVal = spProfile->Read("descr");
        if(true == STR_CMP(pVal->type,typeid(char*).name())||
            true == STR_CMP(pVal->type,typeid(const char*).name()))
        {
            int ratioX,ratioY;
            JIF(m_pMtIn->GetVideoInfo(&m_vmt,&m_width,&m_height,&ratioX,&ratioY));

            const char* descr = (const char*)pVal->value;
            AVFilter *buffersrc,*buffersink;
            AVFilterInOut *inputs,*outputs;
            JCHK(buffersrc  = avfilter_get_by_name("buffer"),E_FAIL);
            JCHK(buffersink = avfilter_get_by_name("buffersink"),E_FAIL);
            JCHK(inputs = avfilter_inout_alloc(),E_FAIL);
            JCHK(outputs  = avfilter_inout_alloc(),E_FAIL);
            JCHK(m_graph = avfilter_graph_alloc(),E_FAIL);

            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            char args[512];
            snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", m_width,m_height,m_vmt, 1, 10000000,ratioX,ratioY);
            JCHK1(0 <= (hr = avfilter_graph_create_filter(&m_ctx_input, buffersrc, "in", args, NULL, m_graph)),E_FAIL,
                "can not create buffer source,msg:%s",av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

            AVBufferSinkParams *buffersink_params;
            enum AVPixelFormat pix_fmts[] = { AVPixelFormat(m_vmt), AV_PIX_FMT_NONE };
            JCHK(buffersink_params = av_buffersink_params_alloc(),E_FAIL);
            buffersink_params->pixel_fmts = pix_fmts;
            JCHK1(0 <= (hr = avfilter_graph_create_filter(&m_ctx_output, buffersink, "out", NULL, buffersink_params, m_graph)),E_FAIL,
                "can not create buffer sink,msg:%s",av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

            /* Endpoints for the filter graph. */
            outputs->name       = av_strdup("in");
            outputs->filter_ctx = m_ctx_input;
            outputs->pad_idx    = 0;
            outputs->next       = NULL;

            inputs->name       = av_strdup("out");
            inputs->filter_ctx = m_ctx_output;
            inputs->pad_idx    = 0;
            inputs->next       = NULL;

            JCHK2(0 <= (hr = avfilter_graph_parse_ptr(m_graph, descr, &inputs, &outputs, NULL)),E_FAIL,
                "avfilter_graph_parse_ptr failed,descr:%s,msg:%s",descr,av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

            JCHK1(0 <= (hr = avfilter_graph_config(m_graph, NULL)),E_FAIL,
                "avfilter_graph_config failed,msg:%s",av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

            m_ctxSws = NULL;
        }
        m_isOpen = true;
    }
	return hr;
}

STDMETHODIMP CFFmpegOSD::Close()
{
    if(true == m_isOpen)
    {
        if(NULL != m_graph)
            avfilter_graph_free(&m_graph);
        if(NULL != m_ctxSws)
        {
            sws_freeContext(m_ctxSws);
            m_ctxSws = NULL;
        }
        m_isOpen = false;
    }
    return S_OK;
}

STDMETHODIMP CFFmpegOSD::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CFFmpegOSD::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegOSD::GetExpend()
{
    return 0;
}

STDMETHODIMP CFFmpegOSD::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    if(NULL == m_pMtOut)
        return E_FAIL;
    else
        return pMT->CopyFrom(m_pMtOut,COPY_FLAG_COMPILATIONS);
}

STDMETHODIMP CFFmpegOSD::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    if(NULL == m_pMtIn)
        return E_FAIL;
    else
        return pMT->CopyFrom(m_pMtIn,COPY_FLAG_COMPILATIONS);
}

STDMETHODIMP CFFmpegOSD::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    HRESULT hr;
    JIF(CheckMediaType(pMT,m_pMtOut));
    m_pMtIn = pMT;
    return S_OK;
}

STDMETHODIMP CFFmpegOSD::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    HRESULT hr;
    JIF(CheckMediaType(m_pMtIn,pMT));
    m_pMtOut = pMT;
    return S_OK;
}

STDMETHODIMP CFFmpegOSD::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
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

STDMETHODIMP CFFmpegOSD::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

HRESULT CFFmpegOSD::Write(IMediaFrame* pFrame)
{
	HRESULT hr = S_OK;
	if(pFrame == NULL)
        return S_STREAM_EOF;
    else if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
        m_spPinOut->NewSegment();

    char err[AV_ERROR_MAX_STRING_SIZE] = {0};

	AVFrame frame;
    memset(&frame,0,sizeof(frame));
    frame.extended_data = frame.data;
    frame.key_frame = 0 == (MEDIA_FRAME_FLAG_SYNCPOINT&pFrame->info.flag) ? 0 : 1;
    frame.pkt_pts = pFrame->info.pts;
    frame.pkt_dts = pFrame->info.dts;
	frame.pts = frame.pkt_pts;

    frame.width = m_width;
	frame.height = m_height;
	frame.format = (int)m_vmt;

	JIF(m_pMtIn->FrameToArray(frame.data,frame.linesize,pFrame));
    JCHK1(0 <= av_buffersrc_add_frame(m_ctx_input,&frame),E_FAIL,"%s",av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
    JCHK1(0 <= av_buffersink_get_frame(m_ctx_output,&frame),E_FAIL,"%s",av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
    VideoMediaType vmt = (VideoMediaType)frame.format;
    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spPinOut->AllocFrame(&spFrame));
    spFrame->info = pFrame->info;
    if(vmt != m_vmt)
    {
        uint8_t *dataOut[AV_NUM_DATA_POINTERS];
		int linesizeOut[AV_NUM_DATA_POINTERS];

        JIF(m_pMtOut->FrameAlloc(spFrame));
        JIF(m_pMtOut->FrameToArray(dataOut,linesizeOut,spFrame));
        if(NULL == m_ctxSws)
        {
            JCHK(m_ctxSws = sws_getContext(m_width,m_height,(AVPixelFormat)vmt,m_width,m_height,(AVPixelFormat)m_vmt,SWS_BICUBIC, NULL, NULL, NULL),E_FAIL);
        }
		char err[AV_ERROR_MAX_STRING_SIZE] = {0};
		//_tprintf(_T("sws_scale:%I64d in\n"),pFrameIn->info.pts);
		JCHK2(m_height == sws_scale(m_ctxSws,frame.data,frame.linesize,0,m_height,dataOut,linesizeOut),
            S_FALSE,"ffmpeg video scale frame[PTS:%l] fail,msg:%s",pFrame->info.pts,
			av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
    }
    else
    {
        JIF(m_pMtOut->ArrayToFrame(spFrame,(const uint8_t**)frame.data,(const int*)frame.linesize));
    }
    av_frame_unref(&frame);
    return m_spPinOut->Write(spFrame);
}

HRESULT CFFmpegOSD::CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr = S_OK;
    MediaSubType type_in,type_out;
    AVPixelFormat pix_fmt_in,pix_fmt_out;
    int width_in,width_out,height_in,height_out;
    int64_t duration_in,duration_out;

    if(NULL != pMtIn)
    {
        if(MST_RAWVIDEO != (type_in = pMtIn->GetSub()))
            return E_INVALIDARG;
        JIF(pMtIn->GetVideoInfo((VideoMediaType*)&pix_fmt_in,&width_in,&height_in,NULL,NULL,&duration_in));
    }

    if(NULL != pMtOut)
    {
        if(MST_RAWVIDEO != (type_out = pMtOut->GetSub()))
            return E_INVALIDARG;
        JIF(pMtOut->GetVideoInfo((VideoMediaType*)&pix_fmt_out,&width_out,&height_out,NULL,NULL,&duration_out));
    }

    if(NULL != pMtIn && NULL != pMtOut)
    {
        if(type_in != type_out || pix_fmt_in != pix_fmt_out || width_in != width_out || height_in != height_out || duration_in != duration_out)
            return E_INVALIDARG;
    }
	return hr;
}

