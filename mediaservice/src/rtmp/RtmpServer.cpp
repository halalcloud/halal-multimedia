#include "RtmpServer.hpp"
#include "RtmpSession.h"
#include "SrsFlvCodec.hpp"

RtmpServer::RtmpServer(CRtmpSession *session)
    : m_session(session)
    , m_type(HandShake)
    , m_in_chunk_size(128)
    , m_out_chunk_size(128)
    , m_player_buffer_length(-1)
    , m_objectEncoding(0)
    , m_stream_id(1)
    , m_first(true)
{
    m_hs = new RtmpHandshake(m_session->m_spStream);
    m_ch = new RtmpChunk(m_session->m_spStream, m_session->m_spAllocate);
    m_req = new rtmp_request();

    // FMLE need
    in_ack_size.window = 2500000;
}

RtmpServer::~RtmpServer()
{
    srs_freep(m_hs);
    srs_freep(m_ch);
    srs_freep(m_req);
    clear();
}

int RtmpServer::service()
{
    HRESULT hr = S_OK;

    while (true) {
        switch (m_type) {
        case HandShake:
            hr = handshake_with_client();
            break;
        case RecvChunk:
            hr = read_chunk();
            break;
        default:
            break;
        }

        if (hr != S_OK) {
            break;
        }
    }

    return hr;
}

int RtmpServer::flush()
{
    return m_ch->flush_chunk_buffer();
}

int RtmpServer::response_connect(bool allow)
{
    HRESULT hr = S_OK;

    if (!allow) {
        JIF(send_connect_refuse());

        return -1;
    }

    JIF_EAGAIN(send_window_ack_size(RTMP_DEFAULT_ACKWINKOW_SIZE));

    JIF(send_connect_response());

    return hr;
}

int RtmpServer::response_publish(bool allow)
{
    HRESULT hr = S_OK;

    if (!allow) {
        return -1;
    }

    JIF_EAGAIN(send_chunk_size(m_out_chunk_size));

    m_ch->set_out_chunk_size(m_out_chunk_size);

    // onFCPublish(NetStream.Publish.Start)
    OnStatusCallPacket* pkt1 = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> temp(pkt1);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt1->setPayload(spFrame);

    pkt1->command_name = RTMP_AMF0_COMMAND_ON_FC_PUBLISH;
    pkt1->header.stream_id = m_stream_id;
    pkt1->value.setValue(StatusCode, new AMF0String(StatusCodePublishStart));
    pkt1->value.setValue(StatusDescription, new AMF0String("Started publishing stream."));

    JIF_EAGAIN(m_ch->send_message(pkt1));

    // onStatus(NetStream.Publish.Start)
    OnStatusCallPacket* pkt2 = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> _temp(pkt2);

    dom_ptr<IMediaFrame> _spFrame;
    JIF(create_frame(&_spFrame));
    pkt2->setPayload(_spFrame);

    pkt2->header.stream_id = m_stream_id;
    pkt2->value.setValue(StatusLevel, new AMF0String(StatusLevelStatus));
    pkt2->value.setValue(StatusCode, new AMF0String(StatusCodePublishStart));
    pkt2->value.setValue(StatusDescription, new AMF0String("Started publishing stream."));
    pkt2->value.setValue(StatusClientId, new AMF0String(RTMP_SIG_CLIENT_ID));

    JIF(m_ch->send_message(pkt2));

    return hr;
}

int RtmpServer::response_play(bool allow)
{
    HRESULT hr = S_OK;

    if (!allow) {
        return -1;
    }

    JIF_EAGAIN(send_chunk_size(m_out_chunk_size));
    m_ch->set_out_chunk_size(m_out_chunk_size);

    // StreamBegin
    UserControlPacket* pkt1 = new UserControlPacket();
    unique_ptr<UserControlPacket> temp(pkt1);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt1->setPayload(spFrame);

    pkt1->event_type = SrcPCUCStreamBegin;
    pkt1->event_data = 1;

    JIF_EAGAIN(m_ch->send_message(pkt1));

    // onStatus(NetStream.Play.Reset)
    OnStatusCallPacket* pkt2 = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> temp_2(pkt2);

    dom_ptr<IMediaFrame> spFrame_2;
    JIF(create_frame(&spFrame_2));
    pkt2->setPayload(spFrame_2);

    pkt2->header.stream_id = m_stream_id;

    pkt2->value.setValue(StatusLevel, new AMF0String(StatusLevelStatus));
    pkt2->value.setValue(StatusCode, new AMF0String(StatusCodeStreamReset));
    pkt2->value.setValue(StatusDescription, new AMF0String("Playing and resetting stream."));
    pkt2->value.setValue(StatusDetails, new AMF0String("stream"));
    pkt2->value.setValue(StatusClientId, new AMF0String(RTMP_SIG_CLIENT_ID));

    JIF_EAGAIN(m_ch->send_message(pkt2));

    // onStatus(NetStream.Play.Start)
    OnStatusCallPacket* pkt3 = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> temp_3(pkt3);

    dom_ptr<IMediaFrame> spFrame_3;
    JIF(create_frame(&spFrame_3));
    pkt3->setPayload(spFrame_3);

    pkt3->header.stream_id = m_stream_id;

    pkt3->value.setValue(StatusLevel, new AMF0String(StatusLevelStatus));
    pkt3->value.setValue(StatusCode, new AMF0String(StatusCodeStreamStart));
    pkt3->value.setValue(StatusDescription, new AMF0String("Started playing stream."));
    pkt3->value.setValue(StatusDetails, new AMF0String("stream"));
    pkt3->value.setValue(StatusClientId, new AMF0String(RTMP_SIG_CLIENT_ID));

    JIF_EAGAIN(m_ch->send_message(pkt3));

    // |RtmpSampleAccess(false, false)
    SampleAccessPacket *pkt4 = new SampleAccessPacket();
    unique_ptr<SampleAccessPacket> temp_4(pkt4);

    dom_ptr<IMediaFrame> spFrame_4;
    JIF(create_frame(&spFrame_4));
    pkt4->setPayload(spFrame_4);

    pkt4->header.stream_id = m_stream_id;

    pkt4->audio_sample_access = true;
    pkt4->video_sample_access = true;

    JIF_EAGAIN(m_ch->send_message(pkt4));

    // onStatus(NetStream.Data.Start)
    OnStatusDataPacket* pkt5 = new OnStatusDataPacket();
    unique_ptr<OnStatusDataPacket> temp_5(pkt5);

    dom_ptr<IMediaFrame> spFrame_5;
    JIF(create_frame(&spFrame_5));
    pkt5->setPayload(spFrame_5);

    pkt5->header.stream_id = m_stream_id;
    pkt5->value.setValue(StatusCode, new AMF0String(StatusCodeDataStart));

    JIF_EAGAIN(m_ch->send_message(pkt5));

    JIF_EAGAIN(send_publish_notify());

    return hr;
}

void RtmpServer::set_chunk_size(int32_t chunk_size)
{
    m_out_chunk_size = chunk_size;
}

int64_t RtmpServer::get_player_buffer_length()
{
    return m_player_buffer_length;
}

int RtmpServer::handshake_with_client()
{
    HRESULT hr = S_OK;

    JIF(m_hs->handshake_with_client());

    if (m_hs->completed()) {
        m_type = RecvChunk;
    }

    return hr;
}

int RtmpServer::read_chunk()
{
    HRESULT hr = S_OK;

    JIF(m_ch->read_chunk());

    if (m_ch->entired()) {
        JIF(decode_message());
    }

    return hr;
}

int RtmpServer::decode_message()
{
    HRESULT hr = S_OK;

    CommonMessage *msg = m_ch->get_message();
    unique_ptr<CommonMessage> temp(msg);

    JIF_EAGAIN(send_acknowledgement());

    if (msg->is_amf0_command() || msg->is_amf3_command()) {
        char *data = (char*)msg->payload->GetBuf()->data;
        int len = msg->size;

        if (msg->is_amf3_command() && msg->size >= 1) {
            data = (char*)msg->payload->GetBuf()->data + 1;
            len = msg->size - 1;
        }

        string command;
        JIF(get_command_name(data, len, command));

        if (command == RTMP_AMF0_COMMAND_CONNECT) {
            return process_connect_app(msg);
        } else if (command == RTMP_AMF0_COMMAND_CREATE_STREAM) {
            return process_create_stream(msg);
        } else if (command == RTMP_AMF0_COMMAND_PUBLISH) {
            return process_publish(msg);
        } else if (command == RTMP_AMF0_COMMAND_RELEASE_STREAM) {
            return process_release_stream(msg);
        } else if (command == RTMP_AMF0_COMMAND_FC_PUBLISH) {
            return process_FCPublish(msg);
        } else if (command == RTMP_AMF0_COMMAND_PLAY) {
            return process_play(msg);
        } else if (command == RTMP_AMF0_COMMAND_CLOSE_STREAM) {
            return process_close_stream(msg);
        } else if (command == RTMP_AMF0_COMMAND_UNPUBLISH) {
            return process_FCUnpublish(msg);
        }
    } else if (msg->is_amf0_data() || msg->is_amf3_data()) {
        char *data = (char*)msg->payload->GetBuf()->data;
        int len = msg->size;

        if (msg->is_amf3_data() && msg->size >= 1) {
            data = (char*)msg->payload->GetBuf()->data + 1;
            len = msg->size - 1;
        }

        string command;
        JIF(get_command_name(data, len, command));

        if(command == RTMP_AMF0_DATA_SET_DATAFRAME || command == RTMP_AMF0_DATA_ON_METADATA) {
            return process_metadata(msg);
        }
    } else if (msg->is_set_chunk_size()) {
        return process_set_chunk_size(msg);
    } else if (msg->is_user_control_message()) {
        return process_user_control(msg);
    } else if (msg->is_window_acknowledgement_size()) {
        return process_window_ackledgement_size(msg);
    } else if (msg->is_video() || msg->is_audio()) {
        return process_video_audio(msg);
    } else if (msg->is_aggregate()) {
        return process_aggregate(msg);
    }

    return hr;
}

int RtmpServer::create_frame(IMediaFrame** ppFrame)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaFrame> spFrame;
    JCHK(spFrame.Create(CLSID_CMediaFrame),E_FAIL);
    JIF(spFrame.CopyTo(ppFrame));

    return hr;
}

int RtmpServer::get_command_name(char *data, int len, string &name)
{
    HRESULT hr = S_OK;

    RtmpPacket *pkt = new RtmpPacket();
    unique_ptr<RtmpPacket> temp(pkt);

    JIF(pkt->decode_packet(data, len));
    if (!pkt->findString(1, name)) {
        return -1;
    }

    return hr;
}

int RtmpServer::process_connect_app(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    ConnectAppPacket *pkt = new ConnectAppPacket();
    unique_ptr<ConnectAppPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    m_tcUrl = pkt->tcUrl;
    m_pageUrl = pkt->pageUrl;
    m_swfUrl = pkt->swfUrl;
    m_objectEncoding = pkt->objectEncoding;

    m_req->set_tcUrl(m_tcUrl);
    m_req->pageUrl = m_pageUrl;
    m_req->swfUrl = m_swfUrl;

    // test
    JIF(response_connect(true));

    return hr;
}

int RtmpServer::process_create_stream(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    CreateStreamPacket *pkt  = new CreateStreamPacket();
    unique_ptr<CreateStreamPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    CreateStreamResPacket *res_pkt = new CreateStreamResPacket(pkt->transaction_id, m_stream_id);
    unique_ptr<CreateStreamResPacket> res_temp(res_pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    res_pkt->setPayload(spFrame);

    JIF(m_ch->send_message(res_pkt));

    return hr;
}

int RtmpServer::process_publish(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    PublishPacket *pkt  = new PublishPacket();
    unique_ptr<PublishPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    if (m_stream_id != pkt->header.stream_id) {
        LOG(0, "rtmp publish stream_id must be %d, actual=%d", m_stream_id, pkt->header.stream_id);
    }

    m_stream_name = pkt->stream_name;

    m_req->set_stream(m_stream_name);

    LOG(0, "name ---> %s, %s, %s", m_req->vhost.c_str(), m_req->app.c_str(), m_req->stream.c_str());

    string url;
    url = m_req->tcUrl;
    size_t pos = url.find('?');
    if(string::npos == pos)
    {
        if('/' != url.back())
            url += '/';
        url += m_req->stream;
    }
    else
    {
        url.insert(pos,m_req->stream);
    }
    CUrl _url;
    JIF(_url.Set(url.c_str()));
    m_session->m_name = _url.GetStreamID(FLV_FORMAT_NAME);
    // test
    JIF(response_publish(true));

    JIF(m_session->SetType(FT_Source));
    JIF(m_session->m_ep->Notify(ET_Session_Push,0,m_session->m_pinOut.p));

    return hr;
}

int RtmpServer::process_play(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    PlayPacket *pkt  = new PlayPacket();
    unique_ptr<PlayPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    if (m_stream_id != pkt->header.stream_id) {
        LOG(0, "rtmp play stream_id must be %d, actual=%d", m_stream_id, pkt->header.stream_id);
    }

    m_stream_name = pkt->stream_name;

    m_req->set_stream(m_stream_name);

    JIF(m_session->SetType(FT_Render));

    string url;
    url = m_req->tcUrl;
    size_t pos = url.find('?');
    if(string::npos == pos)
    {
        if('/' != url.back())
            url += '/';
        url += m_req->stream;
    }
    else
    {
        url.insert(pos,m_req->stream);
    }
    CUrl _url;
    JIF(_url.Set(url.c_str()));
    m_session->m_name = _url.GetStreamID(FLV_FORMAT_NAME);
    // test
    if (m_player_buffer_length != -1) {
        dom_ptr<IMediaType> spMT;
        JCHK(spMT = m_session->m_pinIn->GetMediaType(), E_FAIL);

        dom_ptr<IProfile> spProfile;
        JCHK(spMT.Query(&spProfile), E_FAIL);
        spProfile->Write("buffer_length", m_player_buffer_length);
    }

    JIF(response_play(true));
    JIF(m_session->m_ep->Notify(ET_Session_Pull,0,m_session->m_pinIn.p));

    //m_session->m_status = IFilter::S_Play;
    //JIF(m_session->m_pinIn->Send(IFilter::S_Play, false, true));

    // player buffer length

    return hr;
}

int RtmpServer::process_release_stream(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    FmleStartPacket *pkt  = new FmleStartPacket();
    unique_ptr<FmleStartPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    FmleStartResPacket *pkt1 = new FmleStartResPacket(pkt->transaction_id);
    unique_ptr<FmleStartResPacket> _temp(pkt1);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt1->setPayload(spFrame);

    JIF(m_ch->send_message(pkt1));

    return hr;
}

int RtmpServer::process_FCPublish(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    FmleStartPacket *pkt  = new FmleStartPacket();
    unique_ptr<FmleStartPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    FmleStartResPacket *pkt1 = new FmleStartResPacket(pkt->transaction_id);
    unique_ptr<FmleStartResPacket> _temp(pkt1);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt1->setPayload(spFrame);

    JIF(m_ch->send_message(pkt1));

    return hr;
}

int RtmpServer::process_FCUnpublish(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    FmleStartPacket *pkt  = new FmleStartPacket();
    unique_ptr<FmleStartPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    // response onFCUnpublish(NetStream.unpublish.Success)
    OnStatusCallPacket* pkt1 = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> temp_1(pkt1);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt1->setPayload(spFrame);

    pkt1->command_name = RTMP_AMF0_COMMAND_ON_FC_UNPUBLISH;
    pkt1->value.setValue(StatusCode, new AMF0String(StatusCodeUnpublishSuccess));
    pkt1->value.setValue(StatusDescription, new AMF0String("Stop publishing stream."));

    JIF_EAGAIN(m_ch->send_message(pkt1));

    FmleStartResPacket *pkt2 = new FmleStartResPacket(pkt->transaction_id);
    unique_ptr<FmleStartResPacket> temp_2(pkt2);

    dom_ptr<IMediaFrame> _spFrame;
    JIF(create_frame(&_spFrame));
    pkt2->setPayload(_spFrame);

    JIF(m_ch->send_message(pkt2));

    return hr;
}

int RtmpServer::process_close_stream(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    CloseStreamPacket *pkt  = new CloseStreamPacket();
    unique_ptr<CloseStreamPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    // response onStatus(NetStream.Unpublish.Success)
    OnStatusCallPacket* pkt1 = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> _temp(pkt1);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt1->setPayload(spFrame);

    pkt1->value.setValue(StatusLevel, new AMF0String(StatusLevelStatus));
    pkt1->value.setValue(StatusCode, new AMF0String(StatusCodeUnpublishSuccess));
    pkt1->value.setValue(StatusDescription, new AMF0String("Stream is now unpublished"));
    pkt1->value.setValue(StatusClientId, new AMF0String(RTMP_SIG_CLIENT_ID));

    JIF(m_ch->send_message(pkt1));

    return hr;
}

int RtmpServer::process_set_chunk_size(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    SetChunkSizePacket *pkt  = new SetChunkSizePacket();
    unique_ptr<SetChunkSizePacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    m_in_chunk_size = pkt->chunk_size;
    m_ch->set_in_chunk_size(m_in_chunk_size);

    return hr;
}

int RtmpServer::process_window_ackledgement_size(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    SetWindowAckSizePacket *pkt  = new SetWindowAckSizePacket();
    unique_ptr<SetWindowAckSizePacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    if (pkt->ackowledgement_window_size> 0) {
        in_ack_size.window = pkt->ackowledgement_window_size;
    }

    return hr;
}

int RtmpServer::process_user_control(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    UserControlPacket *pkt  = new UserControlPacket();
    unique_ptr<UserControlPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    if (pkt->event_type == SrcPCUCSetBufferLength) {
        m_player_buffer_length = pkt->extra_data;
    }

    return hr;
}

int RtmpServer::process_metadata(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    OnMetaDataPacket *pkt  = new OnMetaDataPacket();
    unique_ptr<OnMetaDataPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    dom_ptr<IMediaType> spMT;
    JCHK(spMT = m_session->m_pinOut->GetMediaType(), E_FAIL);

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_session->m_pinOut->AllocFrame(&spFrame));
    JIF(convert_to_flv(pkt, spFrame));

    dom_ptr<IMediaFrame> pFrame;
    pFrame = spMT->GetFrame();
    pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, spFrame->GetBuf(0)->size, spFrame->GetBuf(0)->data);

    return hr;
}
//       #include <sys/types.h>
//       #include <sys/stat.h>
//       #include <fcntl.h>
//       #include <unistd.h>

int RtmpServer::process_video_audio(CommonMessage *msg)
{
    HRESULT hr = S_OK;
//    if(INVALID_FD == m_fd)
//    {
//        JCHK2(INVALID_FD != (m_fd = open("session_out.flv",O_WRONLY | O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)),E_FAIL,
//            "open session_out.flv fail,error id:%d message:%s",errno,strerror(errno));
//        JCHK(flv_header_size == write(m_fd,flv_header,flv_header_size),E_FAIL);
//    }
//    dom_ptr<IMediaFrame> spFrame;
//    JIF(m_session->m_pinOut->AllocFrame(&spFrame));
//    JIF(convert_to_flv(msg, spFrame));
//    const IMediaFrame::buf* buf = spFrame->GetBuf();
//    JCHK(buf->size == write(m_fd,buf->data,buf->size),E_FAIL);

    //uint32_t size;
    //IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(msg->payload->GetBuf(0,NULL,&size));
    //uint8_t *buf = pBuf->data;


//    LOG(0,"process_video_audio size:%d p[0-3]%02x %02x %02x %02x",pBuf->size,buf[0],buf[1],buf[2],buf[3]);
    if (m_first) {
        if (m_msgs.size() < 4) {
            CommonMessage *_msg = new CommonMessage(msg);
            m_msgs.push_back(_msg);
        }

        if (m_msgs.size() == 4) {
            return process_av_cache();
        }
    } else {
        JIF(process_av_data(msg));
    }

    return hr;
}

int RtmpServer::process_aggregate(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    if (msg->size <= 0) {
        return hr;
    }

    SrsBuffer stream;

    if (stream.initialize((char*)msg->payload->GetBuf()->data, msg->size) != 0) {
        return -1;
    }

    bool first = true;
    int64_t base_time = 0;

    while (!stream.empty()) {
        JCHK(false == stream.require(1), E_FAIL);
        int8_t type = stream.read_1bytes();

        JCHK(false == stream.require(3), E_FAIL);
        int32_t data_size = stream.read_3bytes();
        if (data_size < 0) {
            return -1;
        }

        JCHK(false == stream.require(3), E_FAIL);
        int32_t timestamp = stream.read_3bytes();

        JCHK(false == stream.require(1), E_FAIL);
        int8_t time_h = stream.read_1bytes();

        timestamp |= (int32_t)time_h << 24;
        timestamp &= 0x7FFFFFFF;

        if (first) {
            base_time = timestamp;
            first = false;
        }

        JCHK(false == stream.require(3), E_FAIL);
        int32_t stream_id = stream.read_3bytes();

        if ((data_size > 0) && (!stream.require(data_size))) {
            return -2;
        }

        CommonMessage _msg;
        _msg.header.message_type = type;
        _msg.header.payload_length = data_size;
        _msg.header.timestamp_delta = timestamp;
        _msg.header.timestamp = msg->header.timestamp + (timestamp - base_time);
        _msg.header.stream_id = stream_id;
        _msg.header.perfer_cid = msg->header.perfer_cid;

        if (data_size > 0) {
            dom_ptr<IMediaFrame> spFrame;
            JIF(create_frame(&spFrame));
            _msg.setPayload(spFrame);

            char *buf = _msg.generate_payload(data_size);
            if (buf == NULL) {
                return -3;
            }

            JCHK(false == stream.require(data_size), E_FAIL);
            stream.read_bytes(buf, data_size);
        }

        JCHK(false == stream.require(4), E_FAIL);

        if (_msg.is_video() || _msg.is_audio()) {
            JIF(process_video_audio(&_msg));
        }
    }

    return hr;
}

int RtmpServer::send_connect_response()
{
    HRESULT hr = S_OK;

    ConnectAppResPacket *pkt = new ConnectAppResPacket();
    unique_ptr<ConnectAppResPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->props.setValue("fmsVer", new AMF0String("FMS/" RTMP_SIG_FMS_VER));
    pkt->props.setValue("capabilities", new AMF0Number(127));
    pkt->props.setValue("mode", new AMF0Number(1));

    pkt->info.setValue(StatusLevel, new AMF0String(StatusLevelStatus));
    pkt->info.setValue(StatusCode, new AMF0String(StatusCodeConnectSuccess));
    pkt->info.setValue(StatusDescription, new AMF0String("Connection succeeded"));
    pkt->info.setValue("objectEncoding", new AMF0Number(m_objectEncoding));

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpServer::send_connect_refuse()
{
    HRESULT hr = S_OK;

    OnErrorPacket *pkt = new OnErrorPacket();
    unique_ptr<OnErrorPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->error_info.setValue(StatusLevel, new AMF0String(StatusLevelError));
    pkt->error_info.setValue(StatusCode, new AMF0String(StatusCodeConnectRejected));
    pkt->error_info.setValue(StatusDescription, new AMF0String("connect refused"));

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpServer::send_window_ack_size(int32_t ack_size)
{
    HRESULT hr = S_OK;

    SetWindowAckSizePacket *pkt = new SetWindowAckSizePacket();
    unique_ptr<SetWindowAckSizePacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->ackowledgement_window_size = ack_size;

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpServer::send_chunk_size(int32_t chunk_size)
{
    HRESULT hr = S_OK;

    SetChunkSizePacket *pkt = new SetChunkSizePacket();
    unique_ptr<SetChunkSizePacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->chunk_size = chunk_size;

    JIF(m_ch->send_message(pkt));

    return hr;

}

int RtmpServer::send_acknowledgement()
{
    HRESULT hr = S_OK;

    if (in_ack_size.window <= 0) {
        return hr;
    }

    IStream::status &status = m_session->m_spStream->GetStatus();
    uint64_t total_recv_size = status.read_total_size;

    // ignore when delta bytes not exceed half of window(ack size).
    uint64_t delta = total_recv_size - in_ack_size.nb_recv_bytes;
    if (delta < (uint64_t)in_ack_size.window / 2) {
        return hr;
    }
    in_ack_size.nb_recv_bytes = total_recv_size;

    // when the sequence number overflow, reset it.
    uint64_t sequence_number = in_ack_size.sequence_number + delta;
    if (sequence_number > 0xf0000000) {
        sequence_number = delta;
    }
    in_ack_size.sequence_number = sequence_number;

    AcknowledgementPacket *pkt = new AcknowledgementPacket();
    unique_ptr<AcknowledgementPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->sequence_number = sequence_number;

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpServer::process_v_sequence(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT = m_session->m_pinOut->GetMediaType(), E_FAIL);

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_session->m_pinOut->AllocFrame(&spFrame));
    JIF(convert_to_flv(msg, spFrame));

    dom_ptr<IMediaFrame> pFrame;
    pFrame = spMT->GetFrame();
    pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, spFrame->GetBuf(0)->size, spFrame->GetBuf(0)->data);

    return hr;
}

int RtmpServer::process_a_sequence(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT = m_session->m_pinOut->GetMediaType(), E_FAIL);

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_session->m_pinOut->AllocFrame(&spFrame));
    JIF(convert_to_flv(msg, spFrame));

    dom_ptr<IMediaFrame> pFrame;
    pFrame = spMT->GetFrame();
    pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, spFrame->GetBuf(0)->size, spFrame->GetBuf(0)->data);

    return hr;
}

int RtmpServer::process_av_cache()
{
    HRESULT hr = S_OK;

    bool has_v_seq = false;
    bool has_a_seq = false;

    list<CommonMessage*>::iterator it;

    for (it = m_msgs.begin(); it != m_msgs.end(); ++it) {
        CommonMessage *msg = *it;

        if (msg->is_video()) {
            IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(msg->payload->GetBuf(0));

            has_v_seq = SrsFlvVideo::sh((char*)pBuf->data, pBuf->size);

            if (has_v_seq) {
                JIF(process_v_sequence(msg));
            } else {
                JIF(process_av_data(msg));
            }
        } else if (msg->is_audio()) {
            IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(msg->payload->GetBuf(0));

            has_a_seq = SrsFlvAudio::sh((char*)pBuf->data, pBuf->size);

            if (has_a_seq) {
                JIF(process_a_sequence(msg));
            } else {
                JIF(process_av_data(msg));
            }
        }
    }

    clear();
    m_first = false;

    JIF(m_session->m_ep->Notify(ET_Filter_Build,0,(IFilter*)m_session));

    return hr;
}

int RtmpServer::process_av_data(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_session->m_pinOut->AllocFrame(&spFrame));
    JIF(convert_to_flv(msg, spFrame));
    spFrame->info.dts *= 10000;
    spFrame->info.pts *= 10000;
    JIF(m_session->m_pinOut->Write(spFrame));

    return hr;
}

void RtmpServer::clear()
{
    list<CommonMessage*>::iterator it;

    for (it = m_msgs.begin(); it != m_msgs.end(); ++it) {
        srs_freep(*it);
    }
    m_msgs.clear();
}

int RtmpServer::send_publish_notify()
{
    HRESULT hr = S_OK;

    OnStatusCallPacket *pkt = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->header.stream_id = m_stream_id;

    std::string description = m_stream_name + " is now published.";

    pkt->value.setValue(StatusLevel, new AMF0String(StatusLevelStatus));
    pkt->value.setValue(StatusCode, new AMF0String(StatusCodeStreamPublishNotify));
    pkt->value.setValue(StatusDescription, new AMF0String(description));
    pkt->value.setValue(StatusDetails, new AMF0String(m_stream_name));
    pkt->value.setValue(StatusClientId, new AMF0String(RTMP_SIG_CLIENT_ID));

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpServer::send_av_message(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    msg->header.stream_id = m_stream_id;

    JIF(m_ch->send_message(msg));

    return hr;
}
