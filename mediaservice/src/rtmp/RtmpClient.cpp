#include "RtmpClient.hpp"
#include "RtmpSession.h"
#include "SrsFlvCodec.hpp"

RtmpClient::RtmpClient(CRtmpSession *session)
    : m_session(session)
    , m_in_chunk_size(128)
    , m_out_chunk_size(128)
    , m_publish(false)
    , m_stream_id(1)
    , m_can_publish(false)
    , m_type(HandShake)
    , m_first(true)
{
    m_hs = new RtmpHandshake(m_session->m_spStream);
    m_hs->set_handshake_type(false);

    m_ch = new RtmpChunk(m_session->m_spStream, m_session->m_spAllocate);
    m_req = new rtmp_request();

    in_ack_size.window = 2500000;
}

RtmpClient::~RtmpClient()
{
    srs_freep(m_hs);
    srs_freep(m_ch);
    srs_freep(m_req);
    clear();
}

void RtmpClient::set_chunk_size(int32_t chunk_size)
{
    m_out_chunk_size = chunk_size;
}

void RtmpClient::set_buffer_length(int64_t len)
{
    m_player_buffer_length = len;
}

bool RtmpClient::can_publish()
{
    return m_can_publish;
}

int RtmpClient::start(bool publish, rtmp_request *req)
{
    HRESULT hr = S_OK;

    m_publish = publish;
    m_req->copy(req);

    JIF(service());

    return hr;
}

int RtmpClient::service()
{
    HRESULT hr = S_OK;

    while (true) {
        switch (m_type) {
        case HandShake:
            hr = handshake_with_server();
            break;
        case Connect:
            hr = connect_app();
            break;
        case CreateStream:
            hr = create_stream();
            break;
        case Publish:
            hr = send_publish();
            break;
        case Play:
            hr = send_play();
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

int RtmpClient::process_connect_response(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    ConnectAppResPacket *pkt = new ConnectAppResPacket();
    unique_ptr<ConnectAppResPacket>temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    string value;
    if (pkt->findString(StatusCode, value)) {
        if (value == StatusCodeConnectClosed || value == StatusCodeConnectRejected) {
            return -1;
        }
    }

    JIF_EAGAIN(send_chunk_size(m_out_chunk_size));

    m_ch->set_out_chunk_size(m_out_chunk_size);

    m_type = CreateStream;

    return hr;
}

int RtmpClient::create_stream()
{
    HRESULT hr = S_OK;

    CreateStreamPacket *pkt = new CreateStreamPacket();
    unique_ptr<CreateStreamPacket>temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    m_requests[pkt->transaction_id] = pkt->command_name;

    m_type = RecvChunk;

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpClient::process_create_stream_response(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    CreateStreamResPacket *pkt = new CreateStreamResPacket(0,0);
    unique_ptr<CreateStreamResPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    m_stream_id = pkt->stream_id;

    if (m_publish) {
        m_type = Publish;
    } else {
        m_type = Play;
    }

    return hr;
}

int RtmpClient::process_command_onstatus(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    OnStatusCallPacket *pkt = new OnStatusCallPacket();
    unique_ptr<OnStatusCallPacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    if (pkt->status_code == StatusCodePublishStart) {
        m_can_publish = true;

        JIF(m_session->SetType(FT_Render));

        m_session->m_status = IFilter::S_Play;
        JIF(m_session->m_pinIn->Send(IFilter::S_Play, false, true));
    }
    if (pkt->status_code == StatusCodeStreamStart) {
        JIF(m_session->SetType(FT_Source));
    }

    m_type = RecvChunk;

    return hr;
}

int RtmpClient::process_set_chunk_size(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    SetChunkSizePacket *pkt = new SetChunkSizePacket();
    unique_ptr<SetChunkSizePacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    m_in_chunk_size = pkt->chunk_size;
    m_ch->set_in_chunk_size(m_in_chunk_size);

    return hr;
}

int RtmpClient::process_window_ackledgement_size(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    SetWindowAckSizePacket *pkt = new SetWindowAckSizePacket();
    unique_ptr<SetWindowAckSizePacket> temp(pkt);
    pkt->copyFrom(msg);

    JIF(pkt->decode());

    if (pkt->ackowledgement_window_size> 0) {
        in_ack_size.window = pkt->ackowledgement_window_size;
    }

    return hr;
}

int RtmpClient::process_metadata(CommonMessage *msg)
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

int RtmpClient::process_video_audio(CommonMessage *msg)
{
    HRESULT hr = S_OK;

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

int RtmpClient::process_aggregate(CommonMessage *msg)
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

int RtmpClient::create_frame(IMediaFrame **ppFrame)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaFrame> spFrame;
    JCHK(spFrame.Create(CLSID_CMediaFrame),E_FAIL);
    JIF(spFrame.CopyTo(ppFrame));

    return hr;
}

int RtmpClient::send_publish()
{
    HRESULT hr = S_OK;

    PublishPacket *pkt = new PublishPacket();
    unique_ptr<PublishPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->header.stream_id = m_stream_id;
    pkt->stream_name = m_req->stream;

    m_type = RecvChunk;

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpClient::send_play()
{
    HRESULT hr = S_OK;

    JIF_EAGAIN(send_buffer_length(m_player_buffer_length));

    PlayPacket *pkt = new PlayPacket();
    unique_ptr<PlayPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->header.stream_id = m_stream_id;
    pkt->stream_name = m_req->stream;

    m_type = RecvChunk;

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpClient::connect_app()
{
    HRESULT hr = S_OK;

    ConnectAppPacket *pkt = new ConnectAppPacket();
    unique_ptr<ConnectAppPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->command_object.setValue("app", new AMF0String(m_req->app));
    pkt->command_object.setValue("flashVer", new AMF0String("WIN 12,0,0,41"));
    pkt->command_object.setValue("swfUrl", new AMF0String(m_req->swfUrl));
    pkt->command_object.setValue("tcUrl", new AMF0String(m_req->tcUrl));
    pkt->command_object.setValue("fpad", new AMF0Boolean(false));
    pkt->command_object.setValue("capabilities", new AMF0Number(239));
    pkt->command_object.setValue("audioCodecs", new AMF0Number(3575));
    pkt->command_object.setValue("videoCodecs", new AMF0Number(252));
    pkt->command_object.setValue("videoFunction", new AMF0Number(1));
    pkt->command_object.setValue("objectEncoding", new AMF0Number(0));

    m_requests[pkt->transaction_id] = pkt->command_name;

    m_type = RecvChunk;

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpClient::handshake_with_server()
{
    HRESULT hr = S_OK;

    JIF(m_hs->handshake_with_server());

    if (m_hs->completed()) {
        m_type = Connect;
    }

    return hr;
}

int RtmpClient::send_chunk_size(int32_t chunk_size)
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

int RtmpClient::send_acknowledgement()
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

int RtmpClient::send_buffer_length(int64_t len)
{
    HRESULT hr = S_OK;

    UserControlPacket *pkt = new UserControlPacket();
    unique_ptr<UserControlPacket> temp(pkt);

    dom_ptr<IMediaFrame> spFrame;
    JIF(create_frame(&spFrame));
    pkt->setPayload(spFrame);

    pkt->event_type = SrcPCUCSetBufferLength;
    pkt->extra_data = len;

    JIF(m_ch->send_message(pkt));

    return hr;
}

int RtmpClient::get_command_name_id(char *data, int len, string &name, double &id)
{
    HRESULT hr = S_OK;

    RtmpPacket *pkt = new RtmpPacket();
    unique_ptr<RtmpPacket> temp(pkt);

    JIF(pkt->decode_packet(data, len));
    if (!pkt->findString(1, name)) {
        return -1;
    }
    if (!pkt->findDouble(1, id)) {
        return -2;
    }

    return hr;
}

int RtmpClient::get_command_name(char *data, int len, string &name)
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

int RtmpClient::read_chunk()
{
    HRESULT hr = S_OK;

    JIF(m_ch->read_chunk());

    if (m_ch->entired()) {
        JIF(decode_message());
    }

    return hr;
}

int RtmpClient::decode_message()
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
        double transactionId;
        JIF(get_command_name_id(data, len, command, transactionId));

        // result/error packet
        if (command == RTMP_AMF0_COMMAND_RESULT || command == RTMP_AMF0_COMMAND_ERROR) {
            // find the call name
            if (m_requests.find(transactionId) != m_requests.end()) {
                string request_name = m_requests[transactionId];

                if (request_name == RTMP_AMF0_COMMAND_CONNECT) {
                    return process_connect_response(msg);
                } else if (request_name == RTMP_AMF0_COMMAND_CREATE_STREAM) {
                    return process_create_stream_response(msg);
                }
            }
        } else if (command == RTMP_AMF0_COMMAND_ON_STATUS) {
            return process_command_onstatus(msg);
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
    } else if (msg->is_window_acknowledgement_size()) {
        return process_window_ackledgement_size(msg);
    } else if (msg->is_video() || msg->is_audio()) {
        return process_video_audio(msg);
    } else if (msg->is_aggregate()) {
        return process_aggregate(msg);
    }

    return hr;
}

int RtmpClient::send_av_message(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    msg->header.stream_id = m_stream_id;

    JIF(m_ch->send_message(msg));

    return hr;
}

int RtmpClient::flush()
{
    return m_ch->flush_chunk_buffer();
}

int RtmpClient::process_v_sequence(CommonMessage *msg)
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

int RtmpClient::process_a_sequence(CommonMessage *msg)
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

int RtmpClient::process_av_cache()
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

int RtmpClient::process_av_data(CommonMessage *msg)
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

void RtmpClient::clear()
{
    list<CommonMessage*>::iterator it;

    for (it = m_msgs.begin(); it != m_msgs.end(); ++it) {
        srs_freep(*it);
    }
    m_msgs.clear();
}
