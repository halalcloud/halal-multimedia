#include "FFmpegDemuxer.h"
const int64_t EXCEPTION_MIN_REV_DURATION = 10000000;
CFFmpegDemuxer::CStream::CStream()
:m_pDemuxer(NULL)
,m_pStream(NULL)
,m_ctxBSF(NULL)
,m_adts(NULL)
,m_pMT(NULL)
,m_duration(0)
,m_input_dts(MEDIA_FRAME_NONE_TIMESTAMP)
,m_delta(MEDIA_FRAME_NONE_TIMESTAMP)
,m_delta_count(0)
,m_isGlobalHeader(false)
,m_start(MEDIA_FRAME_NONE_TIMESTAMP)
,m_length(0)
{
}

CFFmpegDemuxer::CStream::~CStream()
{
    Close();
}

HRESULT CFFmpegDemuxer::CStream::SetStream(CFFmpegDemuxer* pDemuxer,size_t index,AVStream* pStream)
{
    HRESULT hr = S_OK;
	JCHK(NULL != pStream,E_INVALIDARG);
	if(NULL == m_pStream)
	{
        dom_ptr<IMediaType> spMT;
        JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
        if(pStream->time_base.den != 0 && pStream->time_base.num != 0)
        {
            if(AV_NOPTS_VALUE != pStream->start_time)
                m_start = int64_t(pStream->start_time * av_q2d(pStream->time_base) * 10000000);
            if(AV_NOPTS_VALUE == pStream->duration)
            {
                if(MEDIA_FRAME_NONE_TIMESTAMP != pDemuxer->m_ctxFormat->duration)
                {
                    AVRational r = {1,AV_TIME_BASE};
                    m_length = int64_t(pDemuxer->m_ctxFormat->duration * av_q2d(r) * 10000000);;
                }
            }
            else
            {
                m_length = int64_t(pStream->duration * av_q2d(pStream->time_base) * 10000000);
            }
            pStream->codec->time_base = pStream->time_base;
        }

        JIF(spMT->SetSub((MediaSubType)pStream->codec->codec_id));
        if(0 >= pStream->codec->bit_rate)
        {
            LOG(1,"url:%s %sstream-%s can not get bitrate",
                pDemuxer->m_ctxFormat->filename,
                spMT->GetMajorName(),spMT->GetSubName());
        }
        m_isGlobalHeader = true == pDemuxer->m_isGlobalHeader && 0 < pStream->codec->extradata_size;
        if(AVMEDIA_TYPE_VIDEO == pStream->codec->codec_type)
        {
            //pStream->codec->width = 1920;
            //pStream->codec->height = 1080;
            //pStream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
            JCHK5(0 < pStream->codec->width && 0 < pStream->codec->height,E_INVALIDARG,
                "url:%s %sstream-%s width:%d,height:%d is not valid",
                pDemuxer->m_ctxFormat->filename,spMT->GetMajorName(),
                spMT->GetSubName(),pStream->codec->width,pStream->codec->height);
            int ratioX,ratioY;
            if(0 == pStream->codec->sample_aspect_ratio.num || 0 == pStream->codec->sample_aspect_ratio.den)
            {
                if(0 == pStream->sample_aspect_ratio.num || 0 == pStream->sample_aspect_ratio.den)
                {
                    ratioX = pStream->codec->width;
                    ratioY = pStream->codec->height;
                }
                else
                {
                    ratioX = pStream->sample_aspect_ratio.num * pStream->codec->width;
                    ratioY = pStream->sample_aspect_ratio.den * pStream->codec->height;
                }
            }
            else
            {
                ratioX = pStream->codec->sample_aspect_ratio.num * pStream->codec->width;
                ratioY = pStream->codec->sample_aspect_ratio.den * pStream->codec->height;
            }

            int64_t duration;
            if(pStream->r_frame_rate.den != 0 && pStream->r_frame_rate.num != 0)
            {
                duration = int64_t(10000000/av_q2d(pStream->r_frame_rate)+0.5);
            }
            else if(pStream->codec->framerate.den != 0 && pStream->codec->framerate.num != 0)
            {
                duration = int64_t(10000000/av_q2d(pStream->codec->framerate)+0.5);
            }
            else if(pStream->avg_frame_rate.den != 0 && pStream->avg_frame_rate.num != 0)
            {
                duration = int64_t(10000000/av_q2d(pStream->avg_frame_rate)+0.5);
            }
            if(0 == duration)
            {
                duration = 400000;
                LOG(0,"url:%s %sstream-%s can not get fps",
                    pDemuxer->m_ctxFormat->filename,spMT->GetMajorName(),spMT->GetSubName());
            }
            else
            {
                double fps = 10000000.0/duration;
                if(60.0 < fps)
                {
                    LOG(0,"url:%s %sstream-%s fps:%.2f is too big and use default:%.2ffps",
                        pDemuxer->m_ctxFormat->filename,spMT->GetMajorName(),spMT->GetSubName(),
                        fps,25.0);
                    duration = 400000;
                }
            }
            JIF(spMT->SetVideoInfo((VideoMediaType*)&pStream->codec->pix_fmt,
                &pStream->codec->width,&pStream->codec->height,
                &ratioX,&ratioY,&duration));
            m_duration = duration;
            if(true == m_isGlobalHeader)
            {
                if(0 == pStream->codec->extradata[0])
                {
                    LOG(1,"url:%s %sstream-%s global header is invalid",pDemuxer->m_ctxFormat->filename,spMT->GetMajorName(),spMT->GetSubName());
                    m_isGlobalHeader = false;
                }
            }
        }
        else if(AVMEDIA_TYPE_AUDIO == pStream->codec->codec_type)
        {
            if(0 < pStream->codec->channel_layout || 0 < pStream->codec->channels)
            {
                if(0 < pStream->codec->channel_layout)
                    pStream->codec->channels = av_get_channel_layout_nb_channels(pStream->codec->channel_layout);
                else
                    pStream->codec->channel_layout = av_get_default_channel_layout(pStream->codec->channels);
            }
            if(AV_CODEC_ID_WMAPRO == pStream->codec->codec_id)
                pStream->codec->frame_size = 8192;
            JIF(spMT->SetAudioInfo((AudioMediaType*)&pStream->codec->sample_fmt,
                &pStream->codec->channel_layout,&pStream->codec->channels,
                &pStream->codec->sample_rate,&pStream->codec->frame_size));
            m_duration = int64_t((pStream->codec->frame_size * 10000000.0)/pStream->codec->sample_rate+0.5);
        }
        JIF(spMT->SetStreamInfo(&m_start,&m_length,&pStream->codec->bit_rate,
            &m_isGlobalHeader,pStream->codec->extradata,&pStream->codec->extradata_size));
        JCHK(m_spPin.Create(CLSID_COutputPin,(IFilter*)pDemuxer,false,&index),E_FAIL);
        JIF(m_spPin->SetMediaType(spMT));
        m_pMT = spMT.p;
        m_pDemuxer = pDemuxer;
	}
    m_pStream = pStream;
    return hr;
}

bool CFFmpegDemuxer::CStream::IsOutput()
{
    return NULL != m_spPin->GetConnection();
}

HRESULT CFFmpegDemuxer::CStream::Open()
{
    if(m_spPin != NULL)
    {
        m_input_dts = MEDIA_FRAME_NONE_TIMESTAMP;
        m_delta = MEDIA_FRAME_NONE_TIMESTAMP;
        m_delta_count = 0;
    }
    return S_OK;
}

HRESULT CFFmpegDemuxer::CStream::Process(AVPacket& pkt)
{
    if(false == IsOutput())
        return S_FALSE;

    HRESULT hr;
    bool is_first;

    if(AV_NOPTS_VALUE != pkt.pts && pkt.pts < pkt.dts)
    {
        pkt.pts = pkt.dts;
    }
    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spPin->AllocFrame(&spFrame));
    JCHK(S_OK == AVPacketToFrame(spFrame,pkt,m_pStream->time_base,m_pStream->codec->frame_size),S_FALSE);
    int64_t input_dts = spFrame->info.dts;

    if(true == (is_first = (MEDIA_FRAME_NONE_TIMESTAMP == m_input_dts)))
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP == input_dts)
        {
            input_dts = 0;
            spFrame->info.dts = input_dts;
            spFrame->info.pts = input_dts;
        }
        else if(MEDIA_FRAME_NONE_TIMESTAMP == spFrame->info.pts)
        {
            spFrame->info.pts = input_dts;
        }

        if(MEDIA_FRAME_NONE_TIMESTAMP != m_pDemuxer->m_begin)
        {
            int64_t delta = m_pDemuxer->m_begin - input_dts;
            m_pDemuxer->SetDelta(m_delta,delta,0);
        }
    }
    else
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP == input_dts)
        {
            spFrame->info.dts = m_input_dts + m_duration;
            spFrame->info.pts = spFrame->info.dts;
            LOG(1,"input dts is none and adjust to %ldms",spFrame->info.pts/10000);
        }
        else
        {
            if(MEDIA_FRAME_NONE_TIMESTAMP == spFrame->info.pts)
                spFrame->info.pts = spFrame->info.dts;

            int64_t duration_dts = input_dts - m_input_dts;
            if(0 >= duration_dts || duration_dts >  m_duration + 10000000)
            {
                int64_t delta = m_delta + m_duration - duration_dts;
//                const filter_graph_status& status = m_pDemuxer->m_pFG->GetStatus();
//                if(true == status.isLive && MEDIA_FRAME_NONE_TIMESTAMP != status.clockStart)
//                {
//                    int64_t clock = status.timeStart + GetTickCount() - status.clockStart;
//                    int64_t time = input_dts + delta;
//                    if(clock > time)
//                    {
//                        delta += clock - time;
//                        if(m_delta_count == m_pDemuxer->m_delta_count && MEDIA_FRAME_NONE_TIMESTAMP != status.timeOutput)
//                            delta += status.timeInput - status.timeOutput;
//                    }
//                }
                m_delta = delta;
                if(m_delta_count == m_pDemuxer->m_delta_count)
                {
                    m_pDemuxer->m_delta = delta;
                    m_pDemuxer->m_delta_count = ++m_delta_count;
                }
                else if(m_delta_count > m_pDemuxer->m_delta_count)
                {
                    m_pDemuxer->m_delta = delta;
                }
                else
                {
                    if(m_delta < m_pDemuxer->m_delta)
                        m_delta = m_pDemuxer->m_delta;
                    else if(m_delta > m_pDemuxer->m_delta)
                        m_pDemuxer->SetDelta(m_delta);
                    ++m_delta_count;
                }
                LOG(1,"stream[%d:%s-%s] frame dts exception current dts:%ldms previous dts:%ldms duration:%ldms adjust delta:%dms time:%d",
                    m_pStream->index,m_pMT->GetMajorName(),m_pMT->GetSubName(),input_dts/10000,
                    m_input_dts/10000,duration_dts/10000,m_delta/10000,m_delta_count);
            }
            else
            {
                if(m_delta_count < m_pDemuxer->m_delta_count)
                    m_delta_count = m_pDemuxer->m_delta_count;
            }
        }
    }
//    printf("stream[%d:%s-%s] write frame DTS:%ldms PTS:%ldms\n",
//        m_pStream->index,m_pMT->GetMajorName(),m_pMT->GetSubName(),
//        spFrame->info.dts/10000,spFrame->info.pts/10000);
    m_input_dts = input_dts;
    if(MEDIA_FRAME_NONE_TIMESTAMP != m_delta)
    {
        spFrame->info.dts += m_delta;
        spFrame->info.pts += m_delta;
    }
    int64_t time = spFrame->info.dts >= spFrame->info.pts ? spFrame->info.dts : spFrame->info.pts;
    time += m_duration;

    if(time > m_pDemuxer->m_time)
        m_pDemuxer->m_time = time;

    if(NULL == pkt.data || 0 == pkt.size)
    {
        LOG(1,"stream[%d:%s-%s] frame dts:%dms data:%p size:%d is invalid",
            m_pStream->index,m_pMT->GetMajorName(),m_pMT->GetSubName(),int(input_dts/10000),pkt.data,pkt.size);
        return S_FALSE;
    }

    if(0 != (spFrame->info.flag & MEDIA_FRAME_FLAG_CORRUPT))
    {
        LOG(1,"stream[%d:%s-%s] frame data corrupt dts:%dms",
            m_pStream->index,m_pMT->GetMajorName(),m_pMT->GetSubName(),int(spFrame->info.dts/10000));
    }

    if(true == is_first)
    {
        if(true == m_isGlobalHeader)
        {
            bool isGlobalHeader = m_isGlobalHeader;
            m_pMT->GetStreamInfo(NULL,NULL,NULL,&isGlobalHeader,NULL,NULL);
            if(false == isGlobalHeader)
            {
                if(AV_CODEC_ID_H264 == m_pStream->codec->codec_id)
                {
                    if(NULL == m_ctxBSF)
                    {
                        JCHK(m_ctxBSF = av_bitstream_filter_init("h264_mp4toannexb"),E_FAIL);
                    }
                }
                else if(AV_CODEC_ID_HEVC == m_pStream->codec->codec_id)
                {
                    if(NULL == m_ctxBSF)
                    {
                        JCHK(m_ctxBSF = av_bitstream_filter_init("hevc_mp4toannexb"),E_FAIL);
                    }
                }
                else if(AV_CODEC_ID_AAC == m_pStream->codec->codec_id)
                {
                    if(NULL == m_adts)
                    {
                        JCHK(S_OK == adts_decode_extradata(m_pStream->codec->extradata,m_pStream->codec->extradata_size),S_FALSE);
                    }
                }
            }
        }
    }
//    printf("stream[%d:%s-%s] frame dts:%ldms pts:%ldms\n",
//        m_pStream->index,m_pMT->GetMajorName(),m_pMT->GetSubName(),spFrame->info.dts/10000,
//        spFrame->info.pts/10000);
    if(NULL != m_ctxBSF)
    {
        AVPacket pktOut;
        av_init_packet(&pktOut);
        pktOut.data = NULL;
        pktOut.size = 0;
        char err[AV_ERROR_MAX_STRING_SIZE] = {0};

        JCHK3(0 <= (hr=av_bitstream_filter_filter(m_ctxBSF,
					m_pStream->codec,
					NULL,
					&pktOut.data,
					&pktOut.size,
					pkt.data,
					pkt.size,
					pkt.flags & AV_PKT_FLAG_KEY)),E_FAIL,
					"ffmpeg demux stream[%d] frame[DTS:%I64d] add header fail msg:%s",
					m_pStream->index,
					pkt.dts,
					av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

        if(pktOut.data != pkt.data)
        {
            hr = ProcessOut(pktOut,spFrame);
            av_free(pktOut.data);
        }
        else
        {
            hr = ProcessOut(pkt,spFrame);
        }
        return hr;
    }
    else if(NULL != m_adts)
    {
		return adts_write_packet(pkt,spFrame);
    }
    else
    {
        return ProcessOut(pkt,spFrame);
    }
}

void CFFmpegDemuxer::CStream::Close()
{
    if(NULL != m_adts)
    {
        LOG(0,"delete m_adts");
        delete m_adts;
        m_adts = NULL;
    }
    if(NULL != m_ctxBSF)
    {
        LOG(0,"av_bitstream_filter_close");
        av_bitstream_filter_close(m_ctxBSF);
        m_ctxBSF = NULL;
    }
}

HRESULT CFFmpegDemuxer::CStream::ProcessOut(AVPacket& pkt,IMediaFrame* pFrame)
{
    HRESULT hr;
    JIF(pFrame->SetBuf(0,pkt.size,pkt.data));
    JIF(Output(pFrame));
    return hr;
}

HRESULT CFFmpegDemuxer::CStream::adts_decode_extradata(const uint8_t *buf, size_t size)
{
    GetBitContext gb;
    PutBitContext pb;
    MPEG4AudioConfig m4ac;
	memset(&m4ac,0,sizeof(m4ac));
    int off;

    init_get_bits(&gb, buf, size * 8);
    JCHK(0 <= (off = avpriv_mpeg4audio_get_config(&m4ac, buf, size * 8, 1)),E_INVALIDARG);

    skip_bits_long(&gb, off);
    if(NULL == m_adts)
        m_adts = new ADTSContext();
    memset(m_adts,0,sizeof(ADTSContext));
    m_adts->objecttype        = m4ac.object_type - 1;
    m_adts->sample_rate_index = m4ac.sampling_index;
    m_adts->channel_conf      = m4ac.chan_config;

    JCHK1(3U >= m_adts->objecttype,E_INVALIDARG,"MPEG-4 AOT %d is not allowed in ADTS",m_adts->objecttype+1);
    JCHK0(15 != m_adts->sample_rate_index,E_INVALIDARG,"Escape sample rate index illegal in ADTS");
    JCHK0(0 == get_bits(&gb, 1),E_INVALIDARG,"960/120 MDCT window is not allowed in ADTS");
    JCHK0(0 == get_bits(&gb, 1),E_INVALIDARG,"Scalable configurations are not allowed in ADTS");
    JCHK0(0 == get_bits(&gb, 1),E_INVALIDARG,"Extension flag is not allowed in ADTS");

    if (!m_adts->channel_conf) {
        init_put_bits(&pb, m_adts->pce_data, MAX_PCE_SIZE);

        put_bits(&pb, 3, 5); //ID_PCE
        m_adts->pce_size = (avpriv_copy_pce_data(&pb, &gb) + 3) / 8;
        flush_put_bits(&pb);
    }

    m_adts->write_adts = 1;

    return S_OK;
}
#define ADTS_HEADER_SIZE 7
#define ADTS_MAX_FRAME_BYTES ((1 << 13) - 1)

size_t CFFmpegDemuxer::CStream::adts_write_frame_header(uint8_t *buf,size_t size)
{
	PutBitContext pb;
	init_put_bits(&pb, buf, ADTS_HEADER_SIZE);

	/* adts_fixed_header */
	put_bits(&pb, 12, 0xfff);   /* syncword */
	put_bits(&pb, 1, 0);        /* ID */
	put_bits(&pb, 2, 0);        /* layer */
	put_bits(&pb, 1, 1);        /* protection_absent */
	put_bits(&pb, 2, m_adts->objecttype); /* profile_objecttype */
	put_bits(&pb, 4, m_adts->sample_rate_index);
	put_bits(&pb, 1, 0);        /* private_bit */
	put_bits(&pb, 3, m_adts->channel_conf); /* channel_configuration */
	put_bits(&pb, 1, 0);        /* original_copy */
	put_bits(&pb, 1, 0);        /* home */

	/* adts_variable_header */
	put_bits(&pb, 1, 0);        /* copyright_identification_bit */
	put_bits(&pb, 1, 0);        /* copyright_identification_start */
	put_bits(&pb, 13, size); /* aac_frame_length */
	put_bits(&pb, 11, 0x7ff);   /* adts_buffer_fullness */
	put_bits(&pb, 2, 0);        /* number_of_raw_data_blocks_in_frame */
	flush_put_bits(&pb);

	if(0 < m_adts->pce_size)
	{
		memcpy(buf+ADTS_HEADER_SIZE,m_adts->pce_data,m_adts->pce_size);
	}
	return ADTS_HEADER_SIZE + m_adts->pce_size;
}

HRESULT CFFmpegDemuxer::CStream::adts_write_packet(AVPacket& pkt,IMediaFrame* pFrame)
{
    HRESULT hr;
    size_t sz = (unsigned)ADTS_HEADER_SIZE + m_adts->pce_size + pkt.size;
    JIF(pFrame->SetBuf(0,sz));
    const IMediaFrame::buf* buf;
    JCHK(buf = pFrame->GetBuf(),E_FAIL);
    size_t cb = adts_write_frame_header((uint8_t*)buf->data,sz);
    memcpy((uint8_t*)buf->data + cb,pkt.data,pkt.size);
    return Output(pFrame);
}

HRESULT CFFmpegDemuxer::CStream::Output(IMediaFrame* pFrame)
{
    return m_spPin->Write(pFrame);
}

CFFmpegDemuxer::CFFmpegDemuxer()
:m_fd(INVALID_FD)
,m_event(NULL)
,m_msg_event(0)
,m_ctxFormat(NULL)
,m_streams(NULL)
,m_count(0)
,m_isEOF(false)
,m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
,m_time(MEDIA_FRAME_NONE_TIMESTAMP)
,m_delta(MEDIA_FRAME_NONE_TIMESTAMP)
,m_delta_count(0)
,m_begin(MEDIA_FRAME_NONE_TIMESTAMP)
,m_timeout(50000000)
,m_locker(NULL)
{
    //ctor
}

bool CFFmpegDemuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);

    dom_ptr<IEpoll> spEpoll;
    JCHK(spEpoll.QueryFrom(g_pSite->GetObj()),false);
    JCHK(S_OK == spEpoll->CreatePoint(this,&m_epoll),false);
    JCHK(NULL != (m_locker = m_epoll->GetLocker()),false);
    return true;
}

bool CFFmpegDemuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear();
        if(NULL != m_streams)
        {
            delete[] m_streams;
            m_streams = NULL;
        }
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFFmpegDemuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(ISource)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFFmpegDemuxer::GetType()
{
    return FT_Source;
}

STDMETHODIMP CFFmpegDemuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFFmpegDemuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFFmpegDemuxer::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFFmpegDemuxer::GetInputPinCount()
{
    return 0;
}

STDMETHODIMP_(IInputPin*) CFFmpegDemuxer::GetInputPin(uint32_t index)
{
    return NULL;
}

STDMETHODIMP_(uint32_t) CFFmpegDemuxer::GetOutputPinCount()
{
    return m_count;
}

STDMETHODIMP_(IOutputPin*) CFFmpegDemuxer::GetOutputPin(uint32_t index)
{
    JCHK(index < m_count,NULL);
    return m_streams[index].m_spPin;
}

STDMETHODIMP_(IInputPin*) CFFmpegDemuxer::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFFmpegDemuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFFmpegDemuxer::Notify(uint32_t cmd)
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
                m_msg_event = S_Play;
                JCHK(sizeof(m_msg_event) == write(m_fd,&m_msg_event,sizeof(m_msg_event)),E_FAIL);
            }
            else if(S_Pause == cmd)
            {

            }
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    if(S_Play != cmd && S_Pause != cmd)
    {
        uint32_t count = GetOutputPinCount();
        for(uint32_t i=0 ; i<count ; ++i)
        {
            IOutputPin* pPin = GetOutputPin(i);
            if(NULL != pPin->GetConnection())
            {
                JIF(pPin->Send(cmd));
            }
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CFFmpegDemuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFFmpegDemuxer::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFFmpegDemuxer::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFFmpegDemuxer::GetExpend()
{
    return m_te.get();
}

STDMETHODIMP CFFmpegDemuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    return COMPARE_SAME_VALUE == pPinOut->GetMediaType()->Compare(pMT) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CFFmpegDemuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CFFmpegDemuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return E_FAIL;
}

STDMETHODIMP CFFmpegDemuxer::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    JCHK(NULL != pUrl,E_INVALIDARG);
    JCHK(FT_Source == (FilterType)mode,E_INVALIDARG);
    JCHK(false == m_isOpen,E_FAIL);
    JCHK(NULL == m_ctxFormat,E_FAIL);
    HRESULT hr;

	char err[AV_ERROR_MAX_STRING_SIZE] = {0};

	JCHK(m_ctxFormat = avformat_alloc_context(),E_OUTOFMEMORY);

	m_ctxFormat->interrupt_callback.callback = timeout_callback;
    m_ctxFormat->interrupt_callback.opaque = this;
    {
        time_expend te(m_te);
        JCHK2(S_OK <= (hr = avformat_open_input(&m_ctxFormat,pUrl,NULL,NULL)),
            hr,"ffmpeg demuxer open url[%s] fail:%s",pUrl,
            av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
    }

	JCHK2(S_OK <= (hr = avformat_find_stream_info(m_ctxFormat,NULL)),
        hr,"ffmpeg demuxer open url[%s] find stream fail:%s",pUrl,
        av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));

	av_dump_format(m_ctxFormat, 0, "", 0);
    if(0 == strcmp(m_ctxFormat->iformat->name,"mov,mp4,m4a,3gp,3g2,mj2") ||
        0 == strcmp(m_ctxFormat->iformat->name,"flv") ||
        0 == strcmp(m_ctxFormat->iformat->name,"mkv") ||
        0 == strcmp(m_ctxFormat->iformat->name,"3gp"))
        m_isGlobalHeader = true;
    else
        m_isGlobalHeader = false;

	JCHK1(0 < m_ctxFormat->nb_streams,E_INVALIDARG,
        "ffmpeg demuxer can't find stream in url[%s]",pUrl);

    if(m_name != pUrl || m_ctxFormat->nb_streams != m_count)
    {
        if(NULL != m_streams)
        {
            delete[] m_streams;
            m_streams = NULL;
        }
        JCHK(NULL != (m_streams = new CStream[m_ctxFormat->nb_streams]),E_OUTOFMEMORY);
    }

    int64_t bitrate = 0;
    int64_t start_time = MEDIA_FRAME_NONE_TIMESTAMP;
    int64_t end_time = MEDIA_FRAME_NONE_TIMESTAMP;
	for(size_t i = 0 ; i < m_ctxFormat->nb_streams ; ++i)
	{
        JIF(m_streams[i].SetStream(this,i,m_ctxFormat->streams[i]));
        if(MEDIA_FRAME_NONE_TIMESTAMP == start_time || m_streams[i].m_start < start_time)
        {
            start_time = m_streams[i].m_start;
        }
        if(MEDIA_FRAME_NONE_TIMESTAMP != m_streams[i].m_start && 0 < m_streams[i].m_length)
        {
            int64_t end = m_streams[i].m_start + m_streams[i].m_length;
            if(end > end_time)
                end_time = end;
        }
        bitrate += m_streams[i].m_pStream->codec->bit_rate;
	}
	int64_t length = 0;
	if(MEDIA_FRAME_NONE_TIMESTAMP != start_time && MEDIA_FRAME_NONE_TIMESTAMP !=  end_time)
        length = end_time - start_time;


    JCHK(SetFilterInfo(this,&start_time,&length,&bitrate),E_FAIL);

	m_count = m_ctxFormat->nb_streams;
    return SetName(pUrl);
}

//IDemuxer
STDMETHODIMP_(void) CFFmpegDemuxer::SetStartTime(const int64_t& time)
{
    m_begin = time;
}

STDMETHODIMP_(int64_t) CFFmpegDemuxer::GetTime()
{
    return m_time;
}

STDMETHODIMP CFFmpegDemuxer::Process()
{
	HRESULT hr = S_OK;
	while(IS_OK(hr) && E_EOF != hr)
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		{
            time_expend te(m_te);
            hr = av_read_frame(m_ctxFormat, &pkt);
		}
		if(0 <= hr)
		{
//            if(0 != pkt.flags)
//                printf("stream[%d] read pkt dts:%ld pts:%ld flag:%d\n",
//                    pkt.stream_index,pkt.dts,pkt.pts,pkt.flags);
			if(pkt.stream_index < (int)m_count)
			{
                //LOG(0,"demuxer pkt dts:%ld pts:%ld flag:%d",pkt.dts,pkt.pts,pkt.flags);
                hr = m_streams[pkt.stream_index].Process(pkt);
			}
			else
			{
                LOG(1,"url:[%s] demux stream count:%d packet stream index:%d is invalid",
                    m_name.c_str(),m_count,pkt.stream_index);
                hr = S_FALSE;
			}
			av_packet_unref(&pkt);
			if(S_OK == hr)
                return hr;
		}
		else
		{
            if(AVERROR_EOF == hr || -5 == hr)
			{
				LOG(1,"url:[%s] demux end",m_name.c_str());
				hr = E_EOF;
            }
            else
            {
				char err[AV_ERROR_MAX_STRING_SIZE] = {0};
				LOG(1,"url:[%s] demux fail:%s",m_name.c_str(),
                    av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr));
            }
		}
	}
	JIF(Notify(C_Flush));
	return hr;
}

STDMETHODIMP_(void) CFFmpegDemuxer::Clear()
{
    Close();
}

STDMETHODIMP_(bool) CFFmpegDemuxer::IsEOF()
{
    return m_isEOF;
}

STDMETHODIMP_(void) CFFmpegDemuxer::NewSegment()
{
//    for(size_t i = 0 ; i < m_ctxFormat->nb_streams ; ++i)
//        m_streams[i].m_spPin->NewSegment();

}

STDMETHODIMP CFFmpegDemuxer::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(ET_Epoll_Input == type)
    {
        if(0 == m_msg_event)
            read(m_fd,&m_msg_event,sizeof(m_msg_event));
        return Process();
    }
    else
        return E_FAIL;
}

HRESULT CFFmpegDemuxer::Open()
{
    if(true == m_isOpen)
        return S_OK;

    HRESULT hr;

    if(NULL == m_ctxFormat)
    {
        JIF(Load(m_name.c_str(),FT_Source));
    }
	for(size_t i = 0 ; i < m_ctxFormat->nb_streams ; ++i)
	{
        JIF(m_streams[i].Open());
	}

    m_count = m_ctxFormat->nb_streams;
    m_time = MEDIA_FRAME_NONE_TIMESTAMP;
    m_delta = MEDIA_FRAME_NONE_TIMESTAMP;
    m_delta_count = 0;
	m_isEOF = false;
	m_isOpen = true;

	JCHK(INVALID_FD != (m_fd = eventfd(0,EFD_NONBLOCK)),E_FAIL);
    JIF(m_epoll->Add(m_fd));

    LOG(0,"url:[%s] demux open",m_name.c_str());
    return hr;
;
}

HRESULT CFFmpegDemuxer::Close()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
        return hr;

    if(INVALID_FD != m_fd)
    {
        JIF(m_epoll->Del(m_fd));
        close(m_fd);
        m_fd = INVALID_FD;
    }

    if(NULL == m_ctxFormat)
        return hr;
    LOG(0,"url:[%s] m_streams close",m_name.c_str());
    for(size_t i = 0 ; i < m_ctxFormat->nb_streams ; ++i)
    {
        m_streams[i].Close();
    }
    LOG(0,"url:[%s] avformat close",m_name.c_str());
    if(NULL != m_ctxFormat)
    {
        avformat_close_input(&m_ctxFormat);
    }
    if(true == m_isOpen)
    {
        LOG(0,"url:[%s] Notify close event",m_name.c_str());
        m_isOpen = false;
    }
    LOG(0,"url:[%s] demux close",m_name.c_str());
    return hr;
}

void CFFmpegDemuxer::SetDelta(const int64_t& delta)
{
    for(size_t i = 0 ; i < m_ctxFormat->nb_streams ; ++i)
        m_streams[i].m_delta = delta;
    m_delta = delta;
}

void CFFmpegDemuxer::SetDelta(int64_t& o,const int64_t& n,const int64_t& offset)
{
    if(n < m_delta)
        o = m_delta;
    else
    {
        if(n > m_delta)
        {
            if(o < m_delta)
            {
                m_delta = n + offset;
                for(size_t i = 0 ; i < m_ctxFormat->nb_streams ; ++i)
                    m_streams[i].m_delta = m_delta;
            }
            else
                m_delta = n;
        }
        o = n;
    }
}

int CFFmpegDemuxer::timeout_callback(void *pParam)
{
	CFFmpegDemuxer* pThis = (CFFmpegDemuxer*)pParam;
    int64_t begin = pThis->m_te.get_begin();
    if(MEDIA_FRAME_NONE_TIMESTAMP != begin)
    {
        if(pThis->m_timeout <= GetTickCount() - begin)
        {
            LOG(1,"url:[%s] demux block timeout",pThis->m_name.c_str());
            return 1;
        }
    }
	return 0;
}

HRESULT CFFmpegDemuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL == pMtIn)
        return 0;
    return E_INVALIDARG;
}


