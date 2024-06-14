#ifndef RTMPCHUNK_HPP
#define RTMPCHUNK_HPP

#include "stdafx.h"
#include "RtmpGlobal.hpp"
#include "SrsBuffer.hpp"
#include <deque>
#include <map>
#include <Locker.h>

class ChunkStream
{
public:
    ChunkStream()
    {
        extended_timestamp = false;
        msg = NULL;
        first = true;
    }

    ~ChunkStream()
    {
        srs_freep(msg);
    }

public:
    bool first;

    bool extended_timestamp;

    CommonMessageHeader header;
    CommonMessage *msg;
};

class RtmpChunk
{
public:
    RtmpChunk(IStream *socket, IMediaFrameAllocate *allocate);
    ~RtmpChunk();

    int read_chunk();
    int flush_chunk_buffer();

    void set_in_chunk_size(int32_t size);
    void set_out_chunk_size(int32_t size);

    CommonMessage *get_message();

    bool entired();

public:
    int send_message(CommonMessage *msg);

private:
    int read_basic_length_1();
    int read_basic_length_2();
    int read_basic_length_3();

    int read_message_header();
    int read_message_header_extended_timestamp();

    int read_payload_init();
    int read_payload();

    void reset();

private:
    int encode_chunk(CommonMessage *msg);
    int send_chunks(bool flush = true);

private:
    IStream *m_socket;
    IMediaFrameAllocate *m_allocate;
    std::map<int32_t, ChunkStream*> m_chunk_streams;

private:
    int32_t m_in_chunk_size;
    int32_t m_out_chunk_size;

    int8_t m_fmt;
    int32_t m_cid;
    bool m_entire;

    ChunkStream *m_chunk;
    CommonMessage *m_msg;

    int32_t m_payload_size;

    bool m_is_first_chunk_of_msg;

private:
    enum ChunkReadType
    {
        BasicLength_1 = 0,
        BasicLength_2,
        BasicLength_3,
        MessageHeader,
        MessageExtendedTime,
        PayloadInit,
        PayloadRead,
        ChunkFinished
    };
    int8_t m_type;

private:
    CLocker m_locker;

    int send_frame(IMediaFrame *pFrame = NULL);

    typedef list< dom_ptr<IMediaFrame> > FrameSet;
    typedef FrameSet::iterator FrameIt;
    FrameSet m_set;
};

#endif // RTMPCHUNK_HPP
