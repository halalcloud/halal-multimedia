#ifndef RTMPSERVER_HPP
#define RTMPSERVER_HPP

#include "RtmpChunk.hpp"
#include "RtmpGlobal.hpp"
#include "RtmpHandshake.hpp"
#include "RtmpPacket.hpp"
#include "stdafx.h"
#include <list>

class CRtmpSession;

class RtmpServer
{
public:
    RtmpServer(CRtmpSession *session);
    ~RtmpServer();

    int service();

    int flush();

    int response_connect(bool allow);
    int response_publish(bool allow);
    int response_play(bool allow);

    void set_chunk_size(int32_t chunk_size);
    int64_t get_player_buffer_length();

    int send_publish_notify();

    int send_av_message(CommonMessage *msg);

private:
    int handshake_with_client();
    int read_chunk();
    int decode_message();

    int create_frame(IMediaFrame** ppFrame);

    int get_command_name(char *data, int len, string &name);

private:
    int process_connect_app(CommonMessage *msg);

    int process_create_stream(CommonMessage *msg);

    int process_publish(CommonMessage *msg);

    int process_play(CommonMessage *msg);

    int process_release_stream(CommonMessage *msg);

    int process_FCPublish(CommonMessage *msg);

    int process_FCUnpublish(CommonMessage *msg);

    int process_close_stream(CommonMessage *msg);

    int process_set_chunk_size(CommonMessage *msg);

    int process_window_ackledgement_size(CommonMessage *msg);

    int process_user_control(CommonMessage *msg);

    int process_metadata(CommonMessage *msg);

    int process_video_audio(CommonMessage *msg);

    int process_aggregate(CommonMessage *msg);

private:
    int send_connect_response();
    int send_connect_refuse();
    int send_window_ack_size(int32_t ack_size);
    int send_chunk_size(int32_t chunk_size);
    int send_acknowledgement();

private:
    CRtmpSession *m_session;

    RtmpHandshake *m_hs;
    RtmpChunk *m_ch;

    rtmp_request *m_req;

private:
    enum ServerType
    {
        HandShake = 0,
        RecvChunk
    };
    int8_t m_type;

private:
    // 对方发包的chunk size
    int32_t m_in_chunk_size;
    // 我向外发包的chunk size
    int32_t m_out_chunk_size;

    AckWindowSize in_ack_size;

    // 客户端的缓冲区大小
    int64_t m_player_buffer_length;

    double m_objectEncoding;

    // 响应createStream时，发送给客户端，后面的数据全部使用此值，固定为1
    int m_stream_id;

private:
    string m_tcUrl;
    string m_pageUrl;
    string m_swfUrl;

    string m_stream_name;

private:
    int process_v_sequence(CommonMessage *msg);
    int process_a_sequence(CommonMessage *msg);
    int process_av_cache();
    int process_av_data(CommonMessage *msg);

    list<CommonMessage*> m_msgs;
    bool m_first;

    void clear();
};

#endif // RTMPSERVER_HPP
