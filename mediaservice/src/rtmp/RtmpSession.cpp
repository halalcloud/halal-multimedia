#include "RtmpSession.h"
#include <Url.cpp>
#include "SrsFlvCodec.hpp"

CRtmpSession::CRtmpSession()
:m_type(FT_None)
,m_flag(FLAG_LIVE)
,m_status(S_Stop)
,m_pTag(NULL)
,m_server(NULL)
,m_client(NULL)
,m_is_client(false)
,m_correct(true)
{
    memset(&m_stream_status,0,sizeof(m_stream_status));
}

bool CRtmpSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    if(NULL != pParam)
    {
        JCHK(m_spStream.Create(CLSID_CNetworkStream,(ICallback*)this,false,pParam),false);
    }
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    JCHK(m_spAllocate.Create(CLSID_CMediaFrameAllocate),false);

    JCHK(m_jitter = new RtmpTimeJitter(), false);

    m_jitter->set_correct_type(LmsTimeStamp::middle);

    return true;
}

bool CRtmpSession::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        if(S_Play == m_status)
        {
            Notify(S_Pause);
        }
        if(S_Stop != m_status);
        {
            Notify(S_Stop);
        }

        srs_freep(m_server);
        srs_freep(m_client);
        srs_freep(m_jitter);

        LOG(0,"%s[%p] name:%s release",Class().name,(IFilter*)this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CRtmpSession)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CRtmpSession::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    JCHK(FT_Source == mode || FT_Render == mode,E_INVALIDARG);

    HRESULT hr = S_OK;

    if(NULL == pUrl)
        pUrl = m_name.c_str();

    if(m_spStream == NULL)
    {
        CUrl url(pUrl);

        JCHK(m_spStream.Create(CLSID_CNetworkStream,(ICallback*)this),E_FAIL);
        if(0 == url.m_port)
            url.m_port = RTMP_PORT;


        m_name = url.GetStreamID(FLV_FORMAT_NAME);

        if(FT_Source == mode)
        {
            JCHK(m_client = new RtmpClient(this),false);
            m_is_client = true;

            rtmp_request *req = generate_request(&url);

            LOG(0, "---- publish ----> %s, %s, %s, %s", url.m_path.c_str(), req->vhost.c_str(), req->app.c_str(), req->stream.c_str());

            m_client->set_chunk_size(4096);
            m_client->set_buffer_length(3000);
            JIF(m_spStream->Open(url.Get(),(uint32_t)m_type));
            m_client->start(false, req);
        }
        else if (FT_Render == mode)
        {
            JCHK(m_client = new RtmpClient(this),false);
            m_is_client = true;

            rtmp_request *req = generate_request(&url);
            LOG(0, "---- play ----> %s, %s, %s, %s", url.m_path.c_str(), req->vhost.c_str(), req->app.c_str(), req->stream.c_str());

            m_client->set_chunk_size(4096);
            JIF(m_spStream->Open(url.Get(),(uint32_t)m_type));
            m_client->start(true, req);
        }
    }

    return hr;
}

STDMETHODIMP_(FilterType) CRtmpSession::GetType()
{
    return m_type;
}

STDMETHODIMP CRtmpSession::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CRtmpSession::GetName()
{
    return true == m_name.empty() ? NULL : m_name.c_str();
}

STDMETHODIMP_(uint32_t) CRtmpSession::GetFlag()
{
    return m_flag;
}

STDMETHODIMP_(uint32_t) CRtmpSession::GetInputPinCount()
{
    return m_pinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CRtmpSession::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn.p : NULL;
}

STDMETHODIMP_(uint32_t) CRtmpSession::GetOutputPinCount()
{
    return m_pinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CRtmpSession::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut.p : NULL;
}

STDMETHODIMP_(IInputPin*) CRtmpSession::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CRtmpSession::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CRtmpSession::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(FT_Render == m_type)
            {
                if(S_Stop == m_status)
                {
                    m_pinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                }
                else if(S_Stop == cmd)
                {
                    if(m_spStream != NULL)
                        m_spStream->Close();
                }
            }
            else if(FT_Source == m_type)
            {
                if(S_Stop == m_status)
                {
                }
                else if(S_Stop == cmd)
                {
                    if(m_spStream != NULL)
                        m_spStream->Close();
                }
                if(S_Play == cmd)
                {
                    //m_pinOut->NewSegment();
                }
            }
            m_status = (Status)cmd;
            if(FT_Render == m_type)
            {
                if(S_Play == cmd)
                {
                    m_pinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                    JIF(m_ep->Notify(ET_Filter_Buffer,0,m_pinIn.p));
                }
            }
        }
    }
    else
    {
        if(IFilter::C_Profile == cmd)
        {
            //JIF(Init());
        }
        else if(IFilter::C_Accept == cmd)
        {
            JCHK(m_server = new RtmpServer(this),false);
            JIF(m_spStream->SetEventEnable(true));
        }
        else if(IFilter::C_Enable == cmd)
        {
            JIF(m_spStream->SetEventEnable(true));
        }
        else if(IFilter::C_Disable == cmd)
        {
            JIF(m_spStream->SetEventEnable(false));
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CRtmpSession::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CRtmpSession::SetTag(void* tag)
{
    m_pTag = tag;
}

STDMETHODIMP_(void*) CRtmpSession::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CRtmpSession::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CRtmpSession::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    return NULL == pMT || pMT->GetSub() == MST_FLV_TAG ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CRtmpSession::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return NULL == pMT || pMT->GetSub() == MST_FLV_TAG ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CRtmpSession::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    if(false == m_spStream->CanWrite())
        return E_AGAIN;

    HRESULT hr;

    int64_t dts = pFrame->info.dts/10000;

    int64_t jdts = m_jitter->correct(pFrame);

    if (m_correct) {
        dts = jdts;
    }

    if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
    {
//        JCHK(m_spRecord.Create(CLSID_CFileStream),E_FAIL);
//        JIF(m_spRecord->Open("input.flv",FT_Render));

        dom_ptr<IMediaType> spMT;
        JCHK(spMT = m_pinIn->GetMediaType(),E_FAIL);
        dom_ptr<IMediaFrame> spFrame;
        JCHK(spFrame = spMT->GetFrame(),E_FAIL);

        //JIF(m_spRecord->Write(spFrame.p,0,IStream::WRITE_FLAG_FRAME));

        uint32_t count;
        const IMediaFrame::buf* pBuf;

        if(NULL != (pBuf = spFrame->GetBuf(1,&count)))
        {
            for(uint32_t i=0 ;i<count ; ++i)
            {
                JIF(ProcessFlvSequenceHeader(pBuf++, dts));
            }
        }
    }
//    uint32_t sz;
//    pFrame->GetBuf(0,NULL,&sz);
//    JIF(m_spRecord->Write(pFrame,0,IStream::WRITE_FLAG_FRAME));
//    pFrame->info.tag = 2;
    JIF(send_message(pFrame, dts));
    return hr;
}

//IEventCallback
STDMETHODIMP CRtmpSession::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(ET_Stream_Read == type)
        return Recv();
    else if(ET_Stream_Write == type)
    {
        HRESULT hr;
        JIF(Send());

        if(S_FALSE == hr || NULL == m_pinIn)
            return E_AGAIN;


        JIF(m_pinIn->Write(NULL));

        param2 = m_pinIn.p;
    }
    else if(ET_Epoll_Timer == type)
    {
        uint64_t start_ms = *(uint64_t*)param2;
        uint64_t clock_ms = *(uint64_t*)param3;
        IStream::status& status = m_spStream->GetStatus();
        if(start_ms < clock_ms)
        {
            if(m_type == FT_Source)
            {
                if(status.read_total_size == m_stream_status.read_total_size)
                {
                    LOG(0,"rtmp source read 0 byte in 1 second");
                }
            }
            else if(m_type == FT_Render)
            {
                if(status.write_total_size == m_stream_status.write_total_size)
                {
                    LOG(0,"rtmp render write 0 byte in 1 second");
                }
            }
        }
        m_stream_status = status;
    }
    return m_ep->Notify(type,param1,param2,param3);
}

HRESULT CRtmpSession::SetType(FilterType type)
{
    JCHK(FT_Source == type || FT_Render == type,E_INVALIDARG);
    JCHK(FT_None == m_type,E_FAIL);

    HRESULT hr = S_OK;
    if(type == m_type)
        return hr;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
    JIF(spMT->SetSub(MST_FLV_TAG));

    if(FT_Source == type)
    {
        JCHK(m_pinOut.Create(CLSID_COutputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinOut->SetMediaType(spMT));

        dom_ptr<IMediaFrame> spFrame;
        JCHK(spFrame.Create(CLSID_CMediaFrame), E_FAIL);
        JIF(spFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, flv_header_size, flv_header));
        JIF(spMT->SetFrame(spFrame));
        spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
    }
    else if(FT_Render == type)
    {
        JCHK(m_pinIn.Create(CLSID_CInputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinIn->SetMediaType(spMT));
    }
    m_type = type;
    return hr;
}

HRESULT CRtmpSession::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(STR_CMP(protocol,RTMP_PROTOCOL_NAME))
        return 1;
    return E_INVALIDARG;
}

HRESULT CRtmpSession::CreateListener(IStreamListen** ppListen,Interface* pObj,uint16_t* pPort)
{
    if(NULL != ppListen)
    {
        dom_ptr<IStreamListen> spListen;
        JCHK(spListen.Create(CLSID_CNetworkListen,NULL,false,pObj),E_FAIL);
        spListen.CopyTo(ppListen);
    }
    if(NULL != pPort)
    {
        *pPort = RTMP_PORT;
    }
    return 1;
}

int32_t CRtmpSession::Recv()
{
    if (m_is_client) {
        return m_client->service();
    }

    return m_server->service();
}

int32_t CRtmpSession::Send()
{
    if (m_is_client) {
        return m_client->flush();
    }

    return m_server->flush();
}

int32_t CRtmpSession::send_message(IMediaFrame *pFrame, int64_t dts)
{
    HRESULT hr = S_OK;

    uint32_t size;
    IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(pFrame->GetBuf(0,NULL,&size));
    JIF(ProcessFlvSequenceHeader(pBuf,dts));

//    char *p = (char*)pBuf->data;
//    int8_t type = p[0] & 0x1F;
//
//    CommonMessage msg;
//    msg.header.message_type = type;
//    msg.header.payload_length = size - 15;
//    msg.header.timestamp = dts;
//
//    if (type == 8) {
//        msg.header.perfer_cid = 5;
//    } else if (type == 9) {
//        msg.header.perfer_cid = 4;
//    }
//    //pFrame->info.id = p[11];
//    msg.setPayload(pFrame);
//    msg.size = pBuf->size - 15;
//    msg.header.offset = 11;

//    p = (char*)msg.payload->GetBuf(0)->data;

//    LOG(0,"<<<<<<<<<<<<<<send_message[p11:%02x p12:%02x] write frame [%02x][%02x], %ld, %d"
//        ,p[11],p[12], p[0],p[1], dts, msg.size);

//    JIF(send_av_message(&msg));

    return hr;
}

int32_t CRtmpSession::send_av_message(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    if (m_is_client) {
        hr = m_client->send_av_message(msg);
    } else {
        hr = m_server->send_av_message(msg);
    }

    return hr;
}

rtmp_request *CRtmpSession::generate_request(CUrl *curl)
{
    rtmp_request *req = new rtmp_request();

    req->vhost = curl->m_host;

//    size_t pos = std::string::npos;
//    string path = curl->m_path;

    // path = /live/123.flv | /live | /live/123 | /live.flv
    // path一定以'/'开头
//    if ((pos = path.rfind("/")) != std::string::npos) {
//        req->app = path.substr(0, pos);
//        path = path.substr(pos + 1);
//    }
//
//    if ((pos = path.rfind(".")) != std::string::npos) {
//        req->stream = path.substr(0, pos);
//    } else {
//        req->stream = path;
//    }
    req->app = curl->m_path;
    if(1 == req->app.size())
        req->app.clear();
    else
        req->app = req->app.substr(1,req->app.size()-2);
    req->stream = curl->m_file;

    req->tcUrl = "rtmp://" + req->vhost + "/" + req->app;

    return req;
}

int32_t CRtmpSession::ProcessFlvSequenceHeader(const IMediaFrame::buf* pBuf, int64_t dts)
{
    HRESULT hr = S_OK;

    uint8_t *buf = pBuf->data;

    int8_t type = buf[0] & 0x1F;

    // DataSize UI24
    int32_t size;
    char *p = (char*)&size;
    p[3] = 0;
    p[2] = buf[1];
    p[1] = buf[2];
    p[0] = buf[3];

    JCHK(size > 0, E_FAIL);

    // Timestamp UI24
    int32_t timestamp;
    p = (char*)&timestamp;
    p[2] = buf[4];
    p[1] = buf[5];
    p[0] = buf[6];

    // TimestampExtended UI8
    p[3] = buf[7];

    CommonMessage msg;

    msg.header.message_type = type;
    msg.header.payload_length = size;
    msg.header.timestamp = dts;

    if (type == 8) {
        msg.header.perfer_cid = 5;
    } else if (type == 9 || type==0x12) {
        msg.header.perfer_cid = 4;
    }

    msg.size = size;

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spAllocate->Alloc(&spFrame));
    JIF(spFrame->SetBuf(0, size, buf+11));
 //   spFrame->info.tag = 1;
    msg.setPayload(spFrame);

    JIF(send_av_message(&msg));

    return hr;
}
