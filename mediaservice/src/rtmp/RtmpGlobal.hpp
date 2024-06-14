#ifndef RTMPGLOBAL_HPP
#define RTMPGLOBAL_HPP

#include "stdafx.h"
#include <list>
#include <map>
#include <string>

#include <inttypes.h>
#include <sys/types.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define srs_freep(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \
    (void)0

#define srs_freepa(pa) \
    if (pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void)0

#define srs_min(a, b) (((a) < (b))? (a) : (b))
#define srs_max(a, b) (((a) < (b))? (b) : (a))

#define D_UNUSED(name) (void)name;

void srs_random_generate(char* bytes, int size);
bool srs_bytes_equals(void* pa, void* pb, int size);
bool srs_is_little_endian();

#define RTMP_FMT_TYPE0                          0
#define RTMP_FMT_TYPE1                          1
#define RTMP_FMT_TYPE2                          2
#define RTMP_FMT_TYPE3                          3

#define RTMP_EXTENDED_TIMESTAMP                 0xFFFFFF

#define RTMP_CID_ProtocolControl                0x02
#define RTMP_CID_OverConnection                 0x03
#define RTMP_CID_OverConnection2                0x04
#define RTMP_CID_OverStream                     0x05
#define RTMP_CID_Video                          0x06
#define RTMP_CID_Audio                          0x07
#define RTMP_CID_OverStream2                    0x08

#define RTMP_MSG_SetChunkSize                   0x01
#define RTMP_MSG_AbortMessage                   0x02
#define RTMP_MSG_Acknowledgement                0x03
#define RTMP_MSG_UserControlMessage             0x04
#define RTMP_MSG_WindowAcknowledgementSize      0x05
#define RTMP_MSG_SetPeerBandwidth               0x06
#define RTMP_MSG_AMF3CommandMessage             17 // 0x11
#define RTMP_MSG_AMF0CommandMessage             20 // 0x14
#define RTMP_MSG_AMF0DataMessage                18 // 0x12
#define RTMP_MSG_AMF3DataMessage                15 // 0x0F
#define RTMP_MSG_AudioMessage                   8 // 0x08
#define RTMP_MSG_VideoMessage                   9 // 0x09
#define RTMP_MSG_AggregateMessage               22 // 0x16

#define RTMP_AMF0_COMMAND_CONNECT               "connect"
#define RTMP_AMF0_COMMAND_CREATE_STREAM         "createStream"
#define RTMP_AMF0_COMMAND_CLOSE_STREAM          "closeStream"
#define RTMP_AMF0_COMMAND_PLAY                  "play"
#define RTMP_AMF0_COMMAND_PAUSE                 "pause"
#define RTMP_AMF0_COMMAND_ON_BW_DONE            "onBWDone"
#define RTMP_AMF0_COMMAND_ON_STATUS             "onStatus"
#define RTMP_AMF0_COMMAND_RESULT                "_result"
#define RTMP_AMF0_COMMAND_ERROR                 "_error"
#define RTMP_AMF0_COMMAND_RELEASE_STREAM        "releaseStream"
#define RTMP_AMF0_COMMAND_FC_PUBLISH            "FCPublish"
#define RTMP_AMF0_COMMAND_UNPUBLISH             "FCUnpublish"
#define RTMP_AMF0_COMMAND_PUBLISH               "publish"
#define RTMP_AMF0_DATA_SAMPLE_ACCESS            "|RtmpSampleAccess"
#define RTMP_AMF0_DATA_SET_DATAFRAME            "@setDataFrame"
#define RTMP_AMF0_DATA_ON_METADATA              "onMetaData"

#define RTMP_SIG_FMS_VER                        "3,5,3,888"
#define RTMP_SIG_AMF0_VER                       0
#define RTMP_SIG_CLIENT_ID                      "ASAICiss"

#define StatusLevel                             "level"
#define StatusCode                              "code"
#define StatusDescription                       "description"
#define StatusDetails                           "details"
#define StatusClientId                          "clientid"
// status value
#define StatusLevelStatus                       "status"
// status error
#define StatusLevelError                        "error"
// code value
#define StatusCodeConnectSuccess                "NetConnection.Connect.Success"
#define StatusCodeConnectClosed                 "NetConnection.Connect.Closed"
#define StatusCodeConnectRejected               "NetConnection.Connect.Rejected"
#define StatusCodeStreamReset                   "NetStream.Play.Reset"
#define StatusCodeStreamStart                   "NetStream.Play.Start"
#define StatusCodeStreamPause                   "NetStream.Pause.Notify"
#define StatusCodeStreamUnpause                 "NetStream.Unpause.Notify"
#define StatusCodePublishStart                  "NetStream.Publish.Start"
#define StatusCodeDataStart                     "NetStream.Data.Start"
#define StatusCodeUnpublishSuccess              "NetStream.Unpublish.Success"
#define StatusCodeStreamPublishNotify           "NetStream.Play.PublishNotify"
#define StatusCodeStreamNotFound                "NetStream.Play.StreamNotFound"

// FMLE
#define RTMP_AMF0_COMMAND_ON_FC_PUBLISH         "onFCPublish"
#define RTMP_AMF0_COMMAND_ON_FC_UNPUBLISH       "onFCUnpublish"

#define RTMP_DEFAULT_ACKWINKOW_SIZE (2.5 * 1000 * 1000)

class CommonMessageHeader
{
public:
    CommonMessageHeader()
    {
        message_type = 0;
        payload_length = 0;
        timestamp_delta = 0;
        stream_id = 0;
        timestamp = 0;
        perfer_cid = RTMP_CID_OverConnection;
        offset = 0;
    }

    CommonMessageHeader(const CommonMessageHeader &h)
    {
        message_type = h.message_type;
        stream_id = h.stream_id;
        timestamp = h.timestamp;
        payload_length = h.payload_length;
        timestamp_delta = h.timestamp_delta;
        perfer_cid = h.perfer_cid;
        offset = h.offset;
    }

    CommonMessageHeader & operator=(const CommonMessageHeader &h)
    {
        message_type = h.message_type;
        stream_id = h.stream_id;
        timestamp = h.timestamp;
        payload_length = h.payload_length;
        timestamp_delta = h.timestamp_delta;
        perfer_cid = h.perfer_cid;
        offset = h.offset;

        return *this;
    }

public:
    int8_t message_type;
    int32_t stream_id;
    int64_t timestamp;
    int32_t payload_length;
    int32_t timestamp_delta;
    int32_t perfer_cid;
    int32_t offset;
};

class CommonMessage
{
public:
    CommonMessage()
    {
        size = 0;
    }

    CommonMessage(CommonMessage *msg)
    {
        header = msg->header;
        payload = msg->payload;
        size = msg->size;
    }

    virtual ~CommonMessage() {payload = NULL;}

    virtual void copyFrom(CommonMessage *msg)
    {
        this->header = msg->header;
        this->payload = msg->payload;
        this->size = msg->size;
    }

    virtual int encode() { return 0; }
    virtual int decode() { return 0; }

public:
    bool is_audio() { return header.message_type == RTMP_MSG_AudioMessage; }
    bool is_video() { return header.message_type == RTMP_MSG_VideoMessage; }
    bool is_amf0_command() { return header.message_type == RTMP_MSG_AMF0CommandMessage; }
    bool is_amf0_data() { return header.message_type == RTMP_MSG_AMF0DataMessage; }
    bool is_amf3_command() { return header.message_type == RTMP_MSG_AMF3CommandMessage; }
    bool is_amf3_data() { return header.message_type == RTMP_MSG_AMF3DataMessage; }
    bool is_window_acknowledgement_size() { return header.message_type == RTMP_MSG_WindowAcknowledgementSize; }
    bool is_acknowledgement() { return header.message_type == RTMP_MSG_Acknowledgement; }
    bool is_set_chunk_size() { return header.message_type == RTMP_MSG_SetChunkSize; }
    bool is_user_control_message() { return header.message_type == RTMP_MSG_UserControlMessage; }
    bool is_set_peer_bandwidth() { return header.message_type == RTMP_MSG_SetPeerBandwidth; }
    bool is_aggregate() { return header.message_type == RTMP_MSG_AggregateMessage; }

    void setPayload(IMediaFrame *frame)
    {
        payload = frame;
    }

    char* generate_payload(int len)
    {
        if (payload->SetBuf(0,len) < 0) {
            return NULL;
        }

        size = len;

        return (char*)payload->GetBuf()->data;
    }

public:
    CommonMessageHeader header;
    dom_ptr<IMediaFrame> payload;
    int size;
};

class rtmp_request
{
public:
    rtmp_request() {}
    ~rtmp_request() {}

    void copy(rtmp_request *src)
    {
        vhost = src->vhost;
        app = src->app;
        stream = src->stream;
        tcUrl = src->tcUrl;
        pageUrl = src->pageUrl;
        swfUrl = src->swfUrl;
    }

    void set_tcUrl(const string &value)
    {
        tcUrl = value;

        size_t pos = std::string::npos;
        string url = value;

        if ((pos = url.find("://")) != std::string::npos) {
            schema = url.substr(0, pos);
            url = url.substr(schema.length() + 3);
        }

        if ((pos = url.find("/")) != std::string::npos) {
            host = url.substr(0, pos);
            url = url.substr(host.length() + 1);
        }

        port = "1935";
        if ((pos = host.find(":")) != std::string::npos) {
            port = host.substr(pos + 1);
            host = host.substr(0, pos);
        }

        vhost = host;
        app = url;

        if ((pos = url.find("?vhost=")) != std::string::npos) {
            app = url.substr(0, pos);
            vhost = url.substr(app.length() + 7);
        }
    }

    void set_stream(const string &value)
    {
        size_t pos = std::string::npos;
        string url = value;
        stream = value;

        if ((pos = url.find("?")) != std::string::npos) {
            stream = url.substr(0, pos);
            url = url.substr(stream.length() + 1);
        }

        list<string> args = split(url, "&");
        for (int i = 0; i < (int)args.size(); ++i) {
            list<string> temp = split(at(args,i), "=");
            if (temp.size() == 2) {
                params[at(temp,0)] = at(temp,1);
            }
        }
    }

    string get_stream_url()
    {
        return vhost + "/" + app + "/" + stream;
    }

private:
    list<string> split(const string &src, const string &sep)
    {
        string temp = src;
        list<string> ret;
        if (sep.empty()) {
            return ret;
        }

        while (temp.find(sep) != string::npos) {
            string::size_type index = temp.find(sep);

            string ss = temp.substr(0, index);
            if (!ss.empty()) {
                ret.push_back(ss);
            }
            temp = temp.substr(index + sep.size(), temp.size() - 1);
        }
        if (!temp.empty()) {
            ret.push_back(temp);
        }

        return ret;
    }

    string at(list<string> args, int i)
    {
        list<string>::iterator iter = args.begin();
        for (int c = 0; c < i; ++c) {
            ++iter;
        }
        return *iter;
    }

public:
    string vhost;
    string app;
    string stream;

    string schema;
    string host;
    string port;
    std::map<string,string> params;

    string tcUrl;
    string pageUrl;
    string swfUrl;
};

#define JIF_EAGAIN(x) \
    if (((hr = (x)) < 0) && (hr != E_AGAIN)) return hr;

class AckWindowSize
{
public:
    int32_t window;
    // number of received bytes.
    uint64_t nb_recv_bytes;
    // previous responsed sequence number.
    uint64_t sequence_number;

    AckWindowSize() : window(0), nb_recv_bytes(0), sequence_number(0){}
};

int convert_to_flv(CommonMessage *msg, IMediaFrame *pFrame);


#endif // RTMPGLOBAL_HPP
