#include "SrsFlvCodec.hpp"
#include "bitstreamfilter.h"
#include "stdafx.h"

SrsFlvVideo::SrsFlvVideo()
{

}

SrsFlvVideo::~SrsFlvVideo()
{

}

bool SrsFlvVideo::keyframe(char *data, int size)
{
    // 2bytes required.
    if (size < 1) {
        return false;
    }

    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0F;

    return frame_type == SrsVideoAvcFrameTypeKeyFrame;
}

bool SrsFlvVideo::sh(char *data, int size)
{
    // sequence header only for h264
    if (!h264(data, size)) {
        return false;
    }

    // 2bytes required.
    if (size < 2) {
        return false;
    }

    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0F;

    char avc_packet_type = data[1];

    return frame_type == SrsVideoAvcFrameTypeKeyFrame
    && avc_packet_type == SrsVideoAvcFrameTraitSequenceHeader;
}

bool SrsFlvVideo::h264(char *data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }

    char codec_id = data[0];
    codec_id = codec_id & 0x0F;

    return codec_id == SrsVideoCodecIdAVC;
}

bool SrsFlvVideo::acceptable(char *data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }

    char frame_type = data[0];
    char codec_id = frame_type & 0x0f;
    frame_type = (frame_type >> 4) & 0x0f;

    if (frame_type < 1 || frame_type > 5) {
        return false;
    }

    if (codec_id < 2 || codec_id > 7) {
        return false;
    }

    return true;
}

bool SrsFlvVideo::GetFrameCodecInfo(char *data, int size, int8_t &frame, int8_t &codec)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }

    char frame_type = data[0];
    char codec_id = frame_type & 0x0f;
    frame_type = (frame_type >> 4) & 0x0f;

    if (frame_type < 1 || frame_type > 5) {
        return false;
    }

    if (codec_id < 2 || codec_id > 7) {
        return false;
    }

    frame = frame_type;

    switch (codec_id) {
    case SrsCodecVideoSorensonH263:
        codec = MST_H263;
        break;
    case SrsCodecVideoScreenVideo:

        break;
    case SrsCodecVideoOn2VP6:
        codec = MST_VP6;
        break;
    case SrsCodecVideoOn2VP6WithAlphaChannel:

        break;
    case SrsCodecVideoScreenVideoVersion2:

        break;
    case SrsCodecVideoAVC:
        codec = MST_H264;
        break;
    default:
        break;
    }

    return true;
}

char SrsFlvVideo::GetFlvBodyFirstByte(int8_t frame, int8_t codec)
{
    char data = 0x00;

    if (frame == MEDIA_FRAME_FLAG_SYNCPOINT) {
        data = 0x10;
    } else {
        data = 0x20;
    }

    if (codec == MST_H264) {
        data = data | 0x07;
    } else if (codec == MST_H263) {
        data = data | 0x02;
    } else if (codec == MST_VP6) {
        data = data | 0x04;
    }

    return data;
}

SrsFlvAudio::SrsFlvAudio()
{

}

SrsFlvAudio::~SrsFlvAudio()
{

}

bool SrsFlvAudio::sh(char* data, int size)
{
    // sequence header only for aac
    if (!aac(data, size)) {
        return false;
    }

    // 2bytes required.
    if (size < 2) {
        return false;
    }

    char aac_packet_type = data[1];

    return aac_packet_type == SrsAudioAacFrameTraitSequenceHeader;
}

bool SrsFlvAudio::aac(char* data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }

    char sound_format = data[0];
    sound_format = (sound_format >> 4) & 0x0F;

    return sound_format == SrsAudioCodecIdAAC;
}

/**********************************************************************/

SrsFlvCodec::SrsFlvCodec()
{

}

SrsFlvCodec::~SrsFlvCodec()
{

}

int SrsFlvCodec::get_h264_base_info(char *payload, int size, uint32_t &w, uint32_t &h, uint32_t &codecid)
{
    int ret = -1;

    if (size < 5 || !SrsFlvCodec::video_is_h264(payload, size)) {
        return ret;
    }

    char codec_id = payload[0];
    codec_id = codec_id & 0x0F;
    codecid = codec_id;

    return parseAvcDecodeConfigRecord((uint8_t*)payload + 5, (uint32_t)size - 5, w, h);
}

int SrsFlvCodec::get_aac_base_info(char *payload, int size, int &codecid, int &samplerate, int &samplesize, bool &stereo)
{
    if(size < 2 || !SrsFlvCodec::audio_is_aac(payload, size)) {
        return -1;
    }

    char sound = payload[0];
    char sound_format = (sound >> 4) & 0x0f;
    codecid = sound_format;

    char sound_rate = (sound >> 2) & 0x03;
    switch (sound_rate) {
    case SrsCodecAudioSampleRate5512:
        samplerate = 5512;
        break;
    case SrsCodecAudioSampleRate11025:
        samplerate = 11025;
        break;
    case SrsCodecAudioSampleRate22050:
        samplerate = 22050;
        break;
    case SrsCodecAudioSampleRate44100:
        samplerate = 44100;
        break;
    default:
        samplerate = 44100;
        break;
    }

    char sound_size = (sound >> 1) & 0x01;
    switch (sound_size) {
    case 0:
        samplesize = 8;
        break;
    case 1:
        samplesize = 16;
        break;
    default:
        break;
    }

    char sound_type = sound & 0x01;
    switch (sound_type) {
    case 0:
        stereo = false;
        break;
    case 1:
        stereo = true;
        break;
    default:
        break;
    }

    return 0;
}

bool SrsFlvCodec::video_is_h264(char* data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }

    char codec_id = data[0];
    codec_id = codec_id & 0x0F;

    return codec_id == SrsCodecVideoAVC;
}

bool SrsFlvCodec::audio_is_aac(char *data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }

    char sound_format = data[0];
    sound_format = (sound_format >> 4) & 0x0F;

    return sound_format == SrsCodecAudioAAC;
}
