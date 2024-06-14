#include "RtmpChunk.hpp"

RtmpChunk::RtmpChunk(IStream *socket, IMediaFrameAllocate *allocate)
    : m_socket(socket)
    , m_allocate(allocate)
    , m_in_chunk_size(128)
    , m_out_chunk_size(128)
    , m_fmt(0)
    , m_cid(0)
    , m_entire(false)
    , m_chunk(NULL)
    , m_msg(NULL)
    , m_payload_size(0)
    , m_is_first_chunk_of_msg(true)
    , m_type(BasicLength_1)
{

}

RtmpChunk::~RtmpChunk()
{
    map<int, ChunkStream*>::iterator it;

    for (it = m_chunk_streams.begin(); it != m_chunk_streams.end(); ++it) {
        ChunkStream* stream = it->second;
        srs_freep(stream);
    }

    m_chunk_streams.clear();
}

int RtmpChunk::read_chunk()
{
    HRESULT hr = S_OK;

    m_entire = false;

    switch (m_type) {
    case BasicLength_1:
        hr = read_basic_length_1();
        break;
    case BasicLength_2:
        hr = read_basic_length_2();
        break;
    case BasicLength_3:
        hr = read_basic_length_3();
        break;
    case MessageHeader:
        hr = read_message_header();
        break;
    case MessageExtendedTime:
        hr = read_message_header_extended_timestamp();
        break;
    case PayloadInit:
        hr = read_payload_init();
        break;
    case PayloadRead:
        hr = read_payload();
        break;
    case ChunkFinished:
        reset();
        break;
    default:
        break;
    }

    return hr;
}

int RtmpChunk::flush_chunk_buffer()
{
    return send_frame();
}

void RtmpChunk::set_in_chunk_size(int32_t size)
{
    m_in_chunk_size = size;
}

void RtmpChunk::set_out_chunk_size(int32_t size)
{
    m_out_chunk_size = size;
}

CommonMessage *RtmpChunk::get_message()
{
    return m_msg;
}

bool RtmpChunk::entired()
{
    return m_entire;
}

int RtmpChunk::send_message(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    JIF(msg->encode());

    JIF(encode_chunk(msg));

    return hr;
}

int RtmpChunk::read_basic_length_1()
{
    HRESULT hr = S_OK;

    char data[1];
    JIF(m_socket->Read(data, 1));

    SrsBuffer stream;

    if (stream.initialize(data, 1) != 0) {
        return -1;
    }

    m_fmt = stream.read_1bytes();

    m_cid = m_fmt & 0x3f;
    m_fmt = (m_fmt >> 6) & 0x03;

    if (m_cid > 1) {
        m_type = MessageHeader;
        return 1;
    }

    if (m_cid == 0) {
        m_type = BasicLength_2;
    } else if (m_cid == 1) {
        m_type = BasicLength_3;
    }

    return hr;
}

int RtmpChunk::read_basic_length_2()
{
    HRESULT hr = S_OK;

    char data[1];
    JIF(m_socket->Read(data, 1));

    SrsBuffer stream;

    if (stream.initialize(data, 1) != 0) {
        return -1;
    }

    int8_t cid = stream.read_1bytes();

    m_cid = 64 + cid;

    m_type = MessageHeader;

    return hr;
}

int RtmpChunk::read_basic_length_3()
{
    HRESULT hr = S_OK;

    char data[2];
    JIF(m_socket->Read(data, 2));

    SrsBuffer stream;

    if (stream.initialize(data, 2) != 0) {
        return -1;
    }

    int8_t cid_1 = stream.read_1bytes();
    int8_t cid_2 = stream.read_1bytes();

    m_cid = 64 + cid_1 + cid_2 * 256;

    m_type = MessageHeader;

    return hr;
}

int RtmpChunk::read_message_header()
{
    HRESULT hr = S_OK;

    static char mh_sizes[] = {11, 7, 3, 0};
    int mh_size = mh_sizes[(int)m_fmt];
    char data[11];

    if (mh_size > 0) {
        JIF(m_socket->Read(data, mh_size));
    }

    if (m_chunk_streams.find(m_cid) == m_chunk_streams.end()) {
        m_chunk = m_chunk_streams[m_cid] = new ChunkStream();
        m_chunk->header.perfer_cid = m_cid;
    } else {
        m_chunk = m_chunk_streams[m_cid];
    }

    m_is_first_chunk_of_msg = !m_chunk->msg;

    if (m_chunk->first && m_fmt != RTMP_FMT_TYPE0) {
        if (m_cid != RTMP_CID_ProtocolControl && m_fmt != RTMP_FMT_TYPE1) {
            return -1;
        }
    }

    if (m_chunk->msg && m_fmt == RTMP_FMT_TYPE0) {
        return -2;
    }

    if (!m_chunk->msg) {
        m_chunk->msg = new CommonMessage();
        JIF(m_allocate->Alloc(&m_chunk->msg->payload));
    }

    if (m_fmt <= RTMP_FMT_TYPE2) {
        char *p = data;

        char* pp = (char*)&m_chunk->header.timestamp_delta;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        pp[3] = 0;

        m_chunk->extended_timestamp = (m_chunk->header.timestamp_delta >= RTMP_EXTENDED_TIMESTAMP);

        if (!m_chunk->extended_timestamp) {
            if (m_fmt == RTMP_FMT_TYPE0) {
                m_chunk->header.timestamp = m_chunk->header.timestamp_delta;
            } else {
                m_chunk->header.timestamp += m_chunk->header.timestamp_delta;
            }
        }

        if (m_fmt <= RTMP_FMT_TYPE1) {
            int32_t payload_length = 0;
            pp = (char*)&payload_length;
            pp[2] = *p++;
            pp[1] = *p++;
            pp[0] = *p++;
            pp[3] = 0;

            if (!m_is_first_chunk_of_msg && m_chunk->header.payload_length != payload_length) {
                return -3;
            }

            m_chunk->header.payload_length = payload_length;
            m_chunk->header.message_type = *p++;

            if (m_fmt == RTMP_FMT_TYPE0) {
                pp = (char*)&m_chunk->header.stream_id;
                pp[0] = *p++;
                pp[1] = *p++;
                pp[2] = *p++;
                pp[3] = *p++;
            }
        }
    } else {
        if (m_is_first_chunk_of_msg && !m_chunk->extended_timestamp) {
            m_chunk->header.timestamp += m_chunk->header.timestamp_delta;
        }
    }

    if (m_chunk->extended_timestamp) {
        m_type = MessageExtendedTime;
    } else {
        m_type = PayloadInit;
    }

    m_chunk->first = false;

    return hr;
}

int RtmpChunk::read_message_header_extended_timestamp()
{
    HRESULT hr = S_OK;

    char data[4];
    JIF(m_socket->Read(data, 4, IStream::READ_FLAG_PEEK));

    char *p = data;

    uint32_t timestamp = 0x00;
    char* pp = (char*)&timestamp;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;

    timestamp &= 0x7fffffff;

    uint32_t chunk_timestamp = (uint32_t)m_chunk->header.timestamp;

    if (!m_is_first_chunk_of_msg && chunk_timestamp > 0 && chunk_timestamp != timestamp) {
    } else {
        m_chunk->header.timestamp = timestamp;

        char body[4];
        JIF(m_socket->Read(body, 4));
    }

    m_type = PayloadInit;

    return hr;
}

int RtmpChunk::read_payload_init()
{
    HRESULT hr = S_OK;

    m_chunk->header.timestamp &= 0x7fffffff;
    m_chunk->msg->header = m_chunk->header;

    if (m_chunk->header.payload_length <= 0) {
        srs_freep(m_chunk->msg);
        m_type = ChunkFinished;
        return hr;
    }

    if (!m_chunk->msg->payload->GetBuf()) {
        JIF(m_chunk->msg->payload->SetBuf(0, m_chunk->header.payload_length));
    }

    m_payload_size = m_chunk->header.payload_length - m_chunk->msg->size;
    m_payload_size = srs_min(m_payload_size, m_in_chunk_size);

    m_type = PayloadRead;

    return hr;
}

int RtmpChunk::read_payload()
{
    HRESULT hr = S_OK;

    m_payload_size = m_chunk->header.payload_length - m_chunk->msg->size;
    m_payload_size = srs_min(m_payload_size, m_in_chunk_size);

    JIF(m_socket->Read(m_chunk->msg->payload->GetBuf()->data + m_chunk->msg->size, m_payload_size));
    m_chunk->msg->size += m_payload_size;

    if (m_chunk->header.payload_length == m_chunk->msg->size) {
        m_msg = m_chunk->msg;
        m_chunk->msg = NULL;
        m_entire = true;
    }

    m_type = ChunkFinished;

    return hr;
}

void RtmpChunk::reset()
{
    m_fmt = 0;
    m_cid = 0;
    m_payload_size = 0;
    m_type = BasicLength_1;
}

int RtmpChunk::encode_chunk(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    if (msg->header.perfer_cid < 2) {
        msg->header.perfer_cid = RTMP_CID_ProtocolControl;
    }
    uint32_t count,size;
    const IMediaFrame::buf* buf = msg->payload->GetBuf(0,&count,&size);
    char* p = (char*)buf->data/* + msg->header.offset*/;
    char* s = p;
//    LOG(0,"encode_chunk p0:%02x",p[0]);
//    LOG(0,"encode_chunk in buf size:%d total count:%d size:%d msg size:%d",buf->size,count,size,msg->size);
//    LOG(0,"******************************** [%02x][%02x], %ld, %d, %d"
//        , p[0],p[1], msg->header.timestamp, msg->size, msg->header.message_type);


    char* pp;
    dom_ptr<IMediaFrame> spFrame;//spFrame2;
    dom_ptr<IStream> spStream;
    JIF(m_allocate->Alloc(&spFrame));
    spFrame->info = msg->payload->info;
    JCHK(spFrame.Query(&spStream),E_FAIL);
    JIF(spStream->Write(msg->payload.p,0,IStream::WRITE_FLAG_INTERFACE));

//string mdata;

    do {

        char chunk[16];
        // generate the header.
        char* pheader = chunk;

        if (p == s) {
            // write new chunk stream header, fmt is 0
            *pheader++ = 0x00 | (msg->header.perfer_cid & 0x3F);

            // chunk message header, 11 bytes
            // timestamp, 3bytes, big-endian
            uint32_t timestamp = (uint32_t)msg->header.timestamp;
            if (timestamp >= RTMP_EXTENDED_TIMESTAMP) {
                *pheader++ = 0xFF;
                *pheader++ = 0xFF;
                *pheader++ = 0xFF;
            } else {
                pp = (char*)&timestamp;
                *pheader++ = pp[2];
                *pheader++ = pp[1];
                *pheader++ = pp[0];
            }

            // message_length, 3bytes, big-endian
            int payload_length = msg->size;

            pp = (char*)&payload_length;
//            LOG(0, "---------------------------------  %d, %02x, %02x, %02x", msg->size, pp[0], pp[1], pp[2]);
            *pheader++ = pp[2];
            *pheader++ = pp[1];
            *pheader++ = pp[0];

            // message_type, 1bytes
            *pheader++ = msg->header.message_type;

            // message_length, 3bytes, little-endian
            pp = (char*)&msg->header.stream_id;
            *pheader++ = pp[0];
            *pheader++ = pp[1];
            *pheader++ = pp[2];
            *pheader++ = pp[3];

            // chunk extended timestamp header, 0 or 4 bytes, big-endian
            if(timestamp >= RTMP_EXTENDED_TIMESTAMP){
                pp = (char*)&timestamp;
                *pheader++ = pp[3];
                *pheader++ = pp[2];
                *pheader++ = pp[1];
                *pheader++ = pp[0];
            }
        } else {
            *pheader++ = 0xC0 | (msg->header.perfer_cid & 0x3F);
            uint32_t timestamp = (uint32_t)msg->header.timestamp;
            if(timestamp >= RTMP_EXTENDED_TIMESTAMP){
                pp = (char*)&timestamp;
                *pheader++ = pp[3];
                *pheader++ = pp[2];
                *pheader++ = pp[1];
                *pheader++ = pp[0];
            }
        }
        int payload_size = msg->size - (p - s);
        payload_size = srs_min(payload_size, m_out_chunk_size);

        // always has header
        int header_size = pheader - chunk;
        JCHK(0 < header_size && header_size <= 16,E_FAIL);

        JIF(spStream->Write(chunk,header_size));

        JIF(spStream->Write(p,payload_size,IStream::WRITE_FLAG_REFFER));

//mdata.append(chunk, header_size);
//mdata.append(p, payload_size);
        // test
//        dom_ptr<IMediaFrame> spFrame;
//        JIF(m_allocate->Alloc(&spFrame));
//        spFrame->info = msg->payload->info;
//        spFrame->SetBuf(0, header_size);
//        memcpy(spFrame->GetBuf(0)->data, chunk, header_size);
//        spFrame->SetBuf(1, payload_size);
//        memcpy(spFrame->GetBuf(1)->data, p, payload_size);
//        JIF(send_frame(spFrame));

        // consume sendout bytes when not empty packet.
        p += payload_size;
    } while (p < s + msg->size);
    return send_frame(spFrame);
}

int RtmpChunk::send_frame(IMediaFrame *pFrame)
{
    HRESULT hr = S_OK;
    CLocker locker(m_locker);

    if(NULL != pFrame)
        m_set.push_back(pFrame);

    FrameIt it;
    while(m_set.end() != (it = m_set.begin()))
    {
        dom_ptr<IMediaFrame>& spFrame = *it;
        hr = m_socket->Write(spFrame.p, 0, IStream::WRITE_FLAG_FRAME);
        if(S_OK > hr)
        {
            if(E_AGAIN == hr)
            {
                //LOG(0,"send frame dts:%ldms pts:%ld flag:%d E_AGAIN",spFrame->info.dts,spFrame->info.pts,spFrame->info.flag);
                hr = S_FALSE;
            }
            break;
        }
        //LOG(0,"send frame dts:%ldms pts:%ld flag:%d return %d",spFrame->info.dts,spFrame->info.pts,spFrame->info.flag,hr);
        m_set.erase(it);
    }
    return hr;
}
