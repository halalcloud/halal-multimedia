#include "RtmpGlobal.hpp"
#include "SrsFlvCodec.hpp"
#include "SrsBuffer.hpp"

void srs_random_generate(char* bytes, int size)
{
    static bool _random_initialized = false;
    if (!_random_initialized) {
        srand(0);
        _random_initialized = true;
    }

    for (int i = 0; i < size; i++) {
        // the common value in [0x0f, 0xf0]
        bytes[i] = 0x0f + (rand() % (256 - 0x0f - 0x0f));
    }
}

bool srs_bytes_equals(void* pa, void* pb, int size)
{
    uint8_t* a = (uint8_t*)pa;
    uint8_t* b = (uint8_t*)pb;

    if (!a && !b) {
        return true;
    }

    if (!a || !b) {
        return false;
    }

    for(int i = 0; i < size; i++){
        if(a[i] != b[i]){
            return false;
        }
    }

    return true;
}

bool srs_is_little_endian()
{
    // convert to network(big-endian) order, if not equals,
    // the system is little-endian, so need to convert the int64
    static int little_endian_check = -1;

    if(little_endian_check == -1) {
        union {
            int32_t i;
            int8_t c;
        } little_check_union;

        little_check_union.i = 0x01;
        little_endian_check = little_check_union.c;
    }

    return (little_endian_check == 1);
}

int convert_to_flv(CommonMessage *msg, IMediaFrame *pFrame)
{
    HRESULT hr = S_OK;

    JIF(pFrame->SetBuf(0, msg->size + 4 + 11));

    IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(pFrame->GetBuf(0));

    char buf[11];
    char *p = buf;

    // tag header: tag type.
    if (msg->is_video()) {
        *p++ = 0x09;
    } else if (msg->is_audio()) {
        *p++ = 0x08;
    } else {
        *p++ = 0x12;
    }

    // tag header: tag data size.
    char *pp = (char*)&msg->size;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];

    // tag header: tag timestamp.
    pp = (char*)&msg->header.timestamp;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
    *p++ = pp[3];

    // tag header: stream id always 0.
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    memcpy(pBuf->data, buf, 11);
    memcpy(pBuf->data + 11, msg->payload->GetBuf(0)->data, msg->size);

    // previous tag size.
    char pre[4];
    p = pre;

    int pre_tag_len = msg->size + 11;
    pp = (char*)&pre_tag_len;
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];

    memcpy(pBuf->data + 11 + msg->size, pre, 4);

    pFrame->info.dts = msg->header.timestamp;
    pFrame->info.pts = pFrame->info.dts;

    IMediaFrame::buf* mBuf = const_cast<IMediaFrame::buf*>(msg->payload->GetBuf(0));

    bool is_264 = SrsFlvVideo::h264((char*)mBuf->data, mBuf->size);

    pFrame->info.flag = 0;

    // 要进flvmuxer的数据，tag必须是flv body的第一个字节
    p = (char*)mBuf->data;
    //pFrame->info.tag = (char)p[0];

    if (msg->is_video()) {
        if (is_264) {
            char *buf = (char*)mBuf->data + 2;
            SrsBuffer stream;
            JIF(stream.initialize(buf, 3));
            int composition_time = stream.read_3bytes();
            pFrame->info.pts = pFrame->info.dts + composition_time;
        }

        bool is_keyframe = SrsFlvVideo::keyframe((char*)mBuf->data, mBuf->size);
        if (is_keyframe) {
            pFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
        }
    }
    return hr;
}
