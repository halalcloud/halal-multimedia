#ifndef RTMPPACKET_HPP
#define RTMPPACKET_HPP

#include "SrsBuffer.hpp"
#include "RtmpAmf0.hpp"
#include "RtmpGlobal.hpp"
#include "stdafx.h"

class RtmpPacket
{
public:
    RtmpPacket();
    virtual ~RtmpPacket();

    int decode_packet(char *data, int len);
    int encode_packet(SrsBuffer &data);

    void clear();
    // 直接从m_amfs中查找，num从1开始
    bool findString(int num, string &value);
    bool findDouble(int num, double &value);

    // 从 object | ecmaArray | strictArray 中查找
    bool findString(const string &name, string &value);
    bool findDouble(const string &name, double &value);

    // 替换 object | ecmaArray | strictArray 中的src
    bool replaceString(const string &src, const string &dst);
    bool replaceDouble(const string &src, double dst);

    // 替换m_amfs中的第num个，num从1开始
    bool replaceString(int num, const string &dst);
    bool replaceDouble(int num, double dst);

private:
    bool findStringFromAny(AMF0Any *any, const string &name, string &value);
    bool findDoubleFromAny(AMF0Any *any, const string &name, double &value);

    bool replaceStringToAny(AMF0Any *any, const string &src, const string &dst);
    bool replaceDoubleToAny(AMF0Any *any, const string &src, double dst);

public:
    std::vector<AMF0Any*> values;
};

class ConnectAppPacket : public CommonMessage, public RtmpPacket
{
public:
    ConnectAppPacket();
    ~ConnectAppPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;

    AMF0Object command_object;

    // 编码时不需要
    double objectEncoding;
    string tcUrl;
    string pageUrl;
    string swfUrl;
};

class ConnectAppResPacket : public CommonMessage, public RtmpPacket
{
public:
    ConnectAppResPacket();
    ~ConnectAppResPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    AMF0Object props;
    AMF0Object info;
};

class CreateStreamPacket : public CommonMessage, public RtmpPacket
{
public:
    CreateStreamPacket();
    ~CreateStreamPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
};

class CreateStreamResPacket : public CommonMessage, public RtmpPacket
{
public:
    CreateStreamResPacket(double _transaction_id, double _stream_id = 1);
    ~CreateStreamResPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    double stream_id;
};

/**
 * 7.1. Set Chunk Size
 * Protocol control message 1, Set Chunk Size, is used to notify the
 * peer about the new maximum chunk size.
 */
class SetChunkSizePacket : public CommonMessage
{
public:
    SetChunkSizePacket();
    ~SetChunkSizePacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    int32_t chunk_size;
};

/**
 * 5.3. Acknowledgement (3)
 * The client or the server sends the acknowledgment to the peer after
 * receiving bytes equal to the window size.
 */
class AcknowledgementPacket : public CommonMessage
{
public:
    AcknowledgementPacket();
    ~AcknowledgementPacket();

public:
    int encode();

    int get_size();

public:
    int32_t sequence_number;
};

/**
 * 5.5. Window Acknowledgement Size (5)
 * The client or the server sends this message to inform the peer which
 * window size to use when sending acknowledgment.
 */
class SetWindowAckSizePacket : public CommonMessage
{
public:
    SetWindowAckSizePacket();
    ~SetWindowAckSizePacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    int32_t ackowledgement_window_size;
};

/**
 * 用于出错时响应对方信息
 */
class OnErrorPacket : public CommonMessage
{
public:
    OnErrorPacket();
    ~OnErrorPacket();

public:
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    AMF0Object error_info;
};

/**
* 4.1.2. Call
* The call method of the NetConnection object runs remote procedure
* calls (RPC) at the receiving end. The called RPC name is passed as a
* parameter to the call command.
*
* lms不会主动发送call命令
*/
class CallPacket : public CommonMessage, public RtmpPacket
{
public:
    CallPacket();
    ~CallPacket();

public:
    int decode();

public:
    string command_name;
    double transaction_id;
};

/**
 * 响应call命令
 */
class CallResPacket : public CommonMessage
{
public:
    CallResPacket(double _transaction_id);
    ~CallResPacket();

public:
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    // 从外部传入，不负责释放，由外部释放
    AMF0Any *command_object;
    AMF0Any *response;
};

class CloseStreamPacket : public CommonMessage, public RtmpPacket
{
public:
    CloseStreamPacket();
    ~CloseStreamPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
};

class FmleStartPacket : public CommonMessage, public RtmpPacket
{
public:
    FmleStartPacket();
    ~FmleStartPacket();

public:
    int decode();

public:
    string command_name;
    double transaction_id;
    string stream_name;
};

class FmleStartResPacket : public CommonMessage
{
public:
    FmleStartResPacket(double _transaction_id);
    ~FmleStartResPacket();

public:
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    string stream_name;
};

class PublishPacket : public CommonMessage, public RtmpPacket
{
public:
    PublishPacket();
    ~PublishPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    string stream_name;
    string type;
};

class PlayPacket : public CommonMessage, public RtmpPacket
{
public:
    PlayPacket();
    ~PlayPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    string stream_name;
};

// amf0 command onStatus
class OnStatusCallPacket : public CommonMessage, public RtmpPacket
{
public:
    OnStatusCallPacket();
    ~OnStatusCallPacket();

public:
    int decode();
    int encode();

    int get_size();

public:
    string command_name;
    double transaction_id;
    AMF0Object value;
    string status_code;
};

// amf0 data onStatus
class OnStatusDataPacket : public CommonMessage
{
public:
    OnStatusDataPacket();
    ~OnStatusDataPacket();

public:
    int encode();
    int get_size();

public:
    string command_name;
    AMF0Object value;
};

class SampleAccessPacket : public CommonMessage
{
public:
    SampleAccessPacket();
    ~SampleAccessPacket();

public:
    int encode();
    int get_size();

public:
    string command_name;
    bool video_sample_access;
    bool audio_sample_access;
};

// 3.7. User Control message
enum SrcPCUCEventType
{
    // generally, 4bytes event-data

    /**
    * The server sends this event to notify the client
    * that a stream has become functional and can be
    * used for communication. By default, this event
    * is sent on ID 0 after the application connect
    * command is successfully received from the
    * client. The event data is 4-byte and represents
    * the stream ID of the stream that became
    * functional.
    */
    SrcPCUCStreamBegin              = 0x00,

    /**
    * The server sends this event to notify the client
    * that the playback of data is over as requested
    * on this stream. No more data is sent without
    * issuing additional commands. The client discards
    * the messages received for the stream. The
    * 4 bytes of event data represent the ID of the
    * stream on which playback has ended.
    */
    SrcPCUCStreamEOF                = 0x01,

    /**
    * The server sends this event to notify the client
    * that there is no more data on the stream. If the
    * server does not detect any message for a time
    * period, it can notify the subscribed clients
    * that the stream is dry. The 4 bytes of event
    * data represent the stream ID of the dry stream.
    */
    SrcPCUCStreamDry                = 0x02,

    /**
    * The client sends this event to inform the server
    * of the buffer size (in milliseconds) that is
    * used to buffer any data coming over a stream.
    * This event is sent before the server starts
    * processing the stream. The first 4 bytes of the
    * event data represent the stream ID and the next
    * 4 bytes represent the buffer length, in
    * milliseconds.
    */
    SrcPCUCSetBufferLength          = 0x03, // 8bytes event-data

    /**
    * The server sends this event to notify the client
    * that the stream is a recorded stream. The
    * 4 bytes event data represent the stream ID of
    * the recorded stream.
    */
    SrcPCUCStreamIsRecorded         = 0x04,

    /**
    * The server sends this event to test whether the
    * client is reachable. Event data is a 4-byte
    * timestamp, representing the local server time
    * when the server dispatched the command. The
    * client responds with kMsgPingResponse on
    * receiving kMsgPingRequest.
    */
    SrcPCUCPingRequest              = 0x06,

    /**
    * The client sends this event to the server in
    * response to the ping request. The event data is
    * a 4-byte timestamp, which was received with the
    * kMsgPingRequest request.
    */
    SrcPCUCPingResponse             = 0x07,

    SrcPCUCSWFVerifyRequest         = 0x1a, //1bytes
    SrcPCUCSWFVerifyResponse        = 0x1b  //42bytes
};

/**
* 5.4. User Control Message (4)
*
* for the EventData is 4bytes.
* Stream Begin(=0)              4-bytes stream ID
* Stream EOF(=1)                4-bytes stream ID
* StreamDry(=2)                 4-bytes stream ID
* SetBufferLength(=3)           8-bytes 4bytes stream ID, 4bytes buffer length.
* StreamIsRecorded(=4)          4-bytes stream ID
* PingRequest(=6)               4-bytes timestamp local server time
* PingResponse(=7)              4-bytes timestamp received ping request.
*
* 3.7. User Control message
* +------------------------------+-------------------------
* | Event Type ( 2- bytes ) | Event Data
* +------------------------------+-------------------------
* Figure 5 Pay load for the ‘User Control Message’.
*/
class UserControlPacket : public CommonMessage
{
public:
    UserControlPacket();
    ~UserControlPacket();

public:
    int decode();
    int encode();
    int get_size();

public:
    int16_t event_type;
    int32_t event_data;
    int32_t extra_data;
};

class OnMetaDataPacket : public CommonMessage, public RtmpPacket
{
public:
    OnMetaDataPacket();
    ~OnMetaDataPacket();

public:
    int decode();

    int get_size();

private:
    int encode_body();

public:
    string name;
    int num;
};

#endif // RTMPPACKET_HPP
