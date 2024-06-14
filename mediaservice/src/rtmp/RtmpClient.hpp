#ifndef RTMPCLIENT_HPP
#define RTMPCLIENT_HPP

#include "RtmpChunk.hpp"
#include "RtmpGlobal.hpp"
#include "RtmpHandshake.hpp"
#include "RtmpPacket.hpp"
#include "stdafx.h"

class CRtmpSession;

class RtmpClient
{
public:
    RtmpClient(CRtmpSession *session);
    ~RtmpClient();

public:
    void set_chunk_size(int32_t chunk_size);
    void set_buffer_length(int64_t len);

    bool can_publish();

    int start(bool publish, rtmp_request *req);
    int service();

    int send_av_message(CommonMessage *msg);

    int flush();

private:
    int process_connect_response(CommonMessage *msg);
    int create_stream();

    int process_create_stream_response(CommonMessage *msg);
    int process_command_onstatus(CommonMessage *msg);
    int process_set_chunk_size(CommonMessage *msg);
    int process_window_ackledgement_size(CommonMessage *msg);
    int process_metadata(CommonMessage *msg);
    int process_video_audio(CommonMessage *msg);
    int process_aggregate(CommonMessage *msg);

private:
    int create_frame(IMediaFrame** ppFrame);
    int send_publish();
    int send_play();
    int connect_app();
    int handshake_with_server();

private:
    int send_chunk_size(int32_t chunk_size);
    int send_acknowledgement();
    int send_buffer_length(int64_t len);

    int get_command_name_id(char *data, int len, string &name, double &id);
    int get_command_name(char *data, int len, string &name);

    int read_chunk();
    int decode_message();

private:
    CRtmpSession *m_session;

    RtmpHandshake *m_hs;
    RtmpChunk *m_ch;

    rtmp_request *m_req;

    // 对方的chunk size
    int32_t m_in_chunk_size;
    // 我的chunk size
    int32_t m_out_chunk_size;
    // 自己缓冲区的大小，默认30秒
    int64_t m_player_buffer_length;

    std::map<double, string> m_requests;

    bool m_publish;
    int m_stream_id;
    bool m_can_publish;

    AckWindowSize in_ack_size;

private:
    enum ClientSchedule
    {
        HandShake = 0,
        Connect,
        CreateStream,
        Publish,
        Play,
        RecvChunk
    };
    int8_t m_type;

private:
    int process_v_sequence(CommonMessage *msg);
    int process_a_sequence(CommonMessage *msg);
    int process_av_cache();
    int process_av_data(CommonMessage *msg);

    list<CommonMessage*> m_msgs;
    bool m_first;

    void clear();

};

#endif // RTMPCLIENT_HPP
