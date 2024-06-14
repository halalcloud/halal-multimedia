#ifndef SRSFLVCODEC_HPP
#define SRSFLVCODEC_HPP

#include <string>
#include <vector>
#include <stdint.h>
#include <string.h>

/**
 * The audio codec id.
 * @doc video_file_format_spec_v10_1.pdf, page 76, E.4.2 Audio Tags
 * SoundFormat UB [4]
 * Format of SoundData. The following values are defined:
 *     0 = Linear PCM, platform endian
 *     1 = ADPCM
 *     2 = MP3
 *     3 = Linear PCM, little endian
 *     4 = Nellymoser 16 kHz mono
 *     5 = Nellymoser 8 kHz mono
 *     6 = Nellymoser
 *     7 = G.711 A-law logarithmic PCM
 *     8 = G.711 mu-law logarithmic PCM
 *     9 = reserved
 *     10 = AAC
 *     11 = Speex
 *     14 = MP3 8 kHz
 *     15 = Device-specific sound
 * Formats 7, 8, 14, and 15 are reserved.
 * AAC is supported in Flash Player 9,0,115,0 and higher.
 * Speex is supported in Flash Player 10 and higher.
 */
enum SrsAudioCodecId
{
    // set to the max value to reserved, for array map.
    SrsAudioCodecIdReserved1 = 16,
    SrsAudioCodecIdForbidden = 16,

    // for user to disable audio, for example, use pure video hls.
    SrsAudioCodecIdDisabled = 17,

    SrsAudioCodecIdLinearPCMPlatformEndian = 0,
    SrsAudioCodecIdADPCM = 1,
    SrsAudioCodecIdMP3 = 2,
    SrsAudioCodecIdLinearPCMLittleEndian = 3,
    SrsAudioCodecIdNellymoser16kHzMono = 4,
    SrsAudioCodecIdNellymoser8kHzMono = 5,
    SrsAudioCodecIdNellymoser = 6,
    SrsAudioCodecIdReservedG711AlawLogarithmicPCM = 7,
    SrsAudioCodecIdReservedG711MuLawLogarithmicPCM = 8,
    SrsAudioCodecIdReserved = 9,
    SrsAudioCodecIdAAC = 10,
    SrsAudioCodecIdSpeex = 11,
    SrsAudioCodecIdReservedMP3_8kHz = 14,
    SrsAudioCodecIdReservedDeviceSpecificSound = 15,
};

/**
 * The audio AAC frame trait(characteristic).
 * @doc video_file_format_spec_v10_1.pdf, page 77, E.4.2 Audio Tags
 * AACPacketType IF SoundFormat == 10 UI8
 * The following values are defined:
 *      0 = AAC sequence header
 *      1 = AAC raw
 */
enum SrsAudioAacFrameTrait
{
    // set to the max value to reserved, for array map.
    SrsAudioAacFrameTraitReserved = 2,
    SrsAudioAacFrameTraitForbidden = 2,

    SrsAudioAacFrameTraitSequenceHeader = 0,
    SrsAudioAacFrameTraitRawData = 1,
};

/**
 * The audio sample rate.
 * @see srs_flv_srates and srs_aac_srates.
 * @doc video_file_format_spec_v10_1.pdf, page 76, E.4.2 Audio Tags
 *      0 = 5.5 kHz = 5512 Hz
 *      1 = 11 kHz = 11025 Hz
 *      2 = 22 kHz = 22050 Hz
 *      3 = 44 kHz = 44100 Hz
 * However, we can extends this table.
 * @remark Use srs_flv_srates to convert it.
 */
enum SrsAudioSampleRate
{
    // set to the max value to reserved, for array map.
    SrsAudioSampleRateReserved = 4,
    SrsAudioSampleRateForbidden = 4,

    SrsAudioSampleRate5512 = 0,
    SrsAudioSampleRate11025 = 1,
    SrsAudioSampleRate22050 = 2,
    SrsAudioSampleRate44100 = 3,
};

// SoundFormat UB [4]
// Format of SoundData. The following values are defined:
//     0 = Linear PCM, platform endian
//     1 = ADPCM
//     2 = MP3
//     3 = Linear PCM, little endian
//     4 = Nellymoser 16 kHz mono
//     5 = Nellymoser 8 kHz mono
//     6 = Nellymoser
//     7 = G.711 A-law logarithmic PCM
//     8 = G.711 mu-law logarithmic PCM
//     9 = reserved
//     10 = AAC
//     11 = Speex
//     14 = MP3 8 kHz
//     15 = Device-specific sound
// Formats 7, 8, 14, and 15 are reserved.
// AAC is supported in Flash Player 9,0,115,0 and higher.
// Speex is supported in Flash Player 10 and higher.
enum SrsCodecAudio
{
    // set to the max value to reserved, for array map.
    SrsCodecAudioReserved1                = 16,

    // for user to disable audio, for example, use pure video hls.
    SrsCodecAudioDisabled                   = 17,
    SrsCodecAudioLinearPCMPlatformEndian             = 0,
    SrsCodecAudioADPCM                                 = 1,
    SrsCodecAudioMP3                                 = 2,
    SrsCodecAudioLinearPCMLittleEndian                 = 3,
    SrsCodecAudioNellymoser16kHzMono                 = 4,
    SrsCodecAudioNellymoser8kHzMono                 = 5,
    SrsCodecAudioNellymoser                         = 6,
    SrsCodecAudioReservedG711AlawLogarithmicPCM        = 7,
    SrsCodecAudioReservedG711MuLawLogarithmicPCM    = 8,
    SrsCodecAudioReserved                             = 9,
    SrsCodecAudioAAC                                 = 10,
    SrsCodecAudioSpeex                                 = 11,
    SrsCodecAudioReservedMP3_8kHz                     = 14,
    SrsCodecAudioReservedDeviceSpecificSound         = 15,
};

/**
* the FLV/RTMP supported audio sample rate.
* Sampling rate. The following values are defined:
* 0 = 5.5 kHz = 5512 Hz
* 1 = 11 kHz = 11025 Hz
* 2 = 22 kHz = 22050 Hz
* 3 = 44 kHz = 44100 Hz
*/
enum SrsCodecAudioSampleRate
{
    // set to the max value to reserved, for array map.
    SrsCodecAudioSampleRateReserved                 = 4,

    SrsCodecAudioSampleRate5512                     = 0,
    SrsCodecAudioSampleRate11025                    = 1,
    SrsCodecAudioSampleRate22050                    = 2,
    SrsCodecAudioSampleRate44100                    = 3,
};

/**
 * The video codec id.
 * @doc video_file_format_spec_v10_1.pdf, page78, E.4.3.1 VIDEODATA
 * CodecID UB [4]
 * Codec Identifier. The following values are defined for FLV:
 *      2 = Sorenson H.263
 *      3 = Screen video
 *      4 = On2 VP6
 *      5 = On2 VP6 with alpha channel
 *      6 = Screen video version 2
 *      7 = AVC
 */
enum SrsVideoCodecId
{
    // set to the zero to reserved, for array map.
    SrsVideoCodecIdReserved = 0,
    SrsVideoCodecIdForbidden = 0,
    SrsVideoCodecIdReserved1 = 1,
    SrsVideoCodecIdReserved2 = 9,

    // for user to disable video, for example, use pure audio hls.
    SrsVideoCodecIdDisabled = 8,

    SrsVideoCodecIdSorensonH263 = 2,
    SrsVideoCodecIdScreenVideo = 3,
    SrsVideoCodecIdOn2VP6 = 4,
    SrsVideoCodecIdOn2VP6WithAlphaChannel = 5,
    SrsVideoCodecIdScreenVideoVersion2 = 6,
    SrsVideoCodecIdAVC = 7,
};

/**
 * The video AVC frame trait(characteristic).
 * @doc video_file_format_spec_v10_1.pdf, page79, E.4.3.2 AVCVIDEOPACKET
 * AVCPacketType IF CodecID == 7 UI8
 * The following values are defined:
 *      0 = AVC sequence header
 *      1 = AVC NALU
 *      2 = AVC end of sequence (lower level NALU sequence ender is not required or supported)
 */
enum SrsVideoAvcFrameTrait
{
    // set to the max value to reserved, for array map.
    SrsVideoAvcFrameTraitReserved = 3,
    SrsVideoAvcFrameTraitForbidden = 3,

    SrsVideoAvcFrameTraitSequenceHeader = 0,
    SrsVideoAvcFrameTraitNALU = 1,
    SrsVideoAvcFrameTraitSequenceHeaderEOF = 2,
};

/**
 * The video AVC frame type, such as I/P/B.
 * @doc video_file_format_spec_v10_1.pdf, page78, E.4.3.1 VIDEODATA
 * Frame Type UB [4]
 * Type of video frame. The following values are defined:
 *      1 = key frame (for AVC, a seekable frame)
 *      2 = inter frame (for AVC, a non-seekable frame)
 *      3 = disposable inter frame (H.263 only)
 *      4 = generated key frame (reserved for server use only)
 *      5 = video info/command frame
 */
enum SrsVideoAvcFrameType
{
    // set to the zero to reserved, for array map.
    SrsVideoAvcFrameTypeReserved = 0,
    SrsVideoAvcFrameTypeForbidden = 0,
    SrsVideoAvcFrameTypeReserved1 = 6,

    SrsVideoAvcFrameTypeKeyFrame = 1,
    SrsVideoAvcFrameTypeInterFrame = 2,
    SrsVideoAvcFrameTypeDisposableInterFrame = 3,
    SrsVideoAvcFrameTypeGeneratedKeyFrame = 4,
    SrsVideoAvcFrameTypeVideoInfoFrame = 5,
};

// E.4.3.1 VIDEODATA
// CodecID UB [4]
// Codec Identifier. The following values are defined:
//     2 = Sorenson H.263
//     3 = Screen video
//     4 = On2 VP6
//     5 = On2 VP6 with alpha channel
//     6 = Screen video version 2
//     7 = AVC
enum SrsCodecVideo
{
    // set to the max value to reserved, for array map.
    SrsCodecVideoReserved                = 0,
    SrsCodecVideoReserved1                = 1,
    SrsCodecVideoReserved2                = 9,
    // for user to disable video, for example, use pure audio hls.
    SrsCodecVideoDisabled                = 8,
    SrsCodecVideoSorensonH263             = 2,
    SrsCodecVideoScreenVideo             = 3,
    SrsCodecVideoOn2VP6                 = 4,
    SrsCodecVideoOn2VP6WithAlphaChannel = 5,
    SrsCodecVideoScreenVideoVersion2     = 6,
    SrsCodecVideoAVC                     = 7,
    SrsCodecVideoHEVC                     = 10,
};

class SrsFlvVideo
{
public:
    SrsFlvVideo();
    ~SrsFlvVideo();

public:
    /**
     * only check the frame_type, not check the codec type.
     */
    static bool keyframe(char* data, int size);
    /**
     * check codec h264, keyframe, sequence header
     */
    static bool sh(char* data, int size);
    /**
     * check codec h264.
     */
    static bool h264(char* data, int size);
    /**
     * check the video RTMP/flv header info,
     * @return true if video RTMP/flv header is ok.
     * @remark all type of audio is possible, no need to check audio.
     */
    static bool acceptable(char* data, int size);

    static bool GetFrameCodecInfo(char *data, int size, int8_t &frame, int8_t &codec);
    static char GetFlvBodyFirstByte(int8_t frame, int8_t codec);
};

class SrsFlvAudio
{
public:
    SrsFlvAudio();
    virtual ~SrsFlvAudio();
    // the following function used to finger out the flv/rtmp packet detail.
public:
    /**
     * check codec aac, sequence header
     */
    static bool sh(char* data, int size);
    /**
     * check codec aac.
     */
    static bool aac(char* data, int size);
};

class SrsFlvCodec
{
public:
    SrsFlvCodec();
    virtual ~SrsFlvCodec();

    static int get_h264_base_info(char *payload, int size, uint32_t &w, uint32_t &h, uint32_t &codecid);
    static int get_aac_base_info(char *payload, int size, int &codecid, int &samplerate, int &samplesize, bool &stereo);

    static bool video_is_h264(char* data, int size);
    static bool audio_is_aac(char* data, int size);

};

class aac_adts2asc
{
    uint8_t _dsi[2];
    class AdtsHeader {
        static const int AP4_SUCCESS = 0;
        static const int AP4_FAILURE = -1;
    public:
        // constructor
        AdtsHeader(const uint8_t* bytes)
        {
            // fixed part
            m_Id                     = ( bytes[1] & 0x08) >> 3;
            m_ProtectionAbsent       =   bytes[1] & 0x01;
            m_ProfileObjectType      = ( bytes[2] & 0xC0) >> 6;
            m_SamplingFrequencyIndex = ( bytes[2] & 0x3C) >> 2;
            m_ChannelConfiguration   = ((bytes[2] & 0x01) << 2) |
                    ((bytes[3] & 0xC0) >> 6);
            // variable part
            m_FrameLength = ((unsigned int)(bytes[3] & 0x03) << 11) |
                    ((unsigned int)(bytes[4]       ) <<  3) |
                    ((unsigned int)(bytes[5] & 0xE0) >>  5);
            m_RawDataBlocks =               bytes[6] & 0x03;
        }

        // methods
        int Check()
        {
            // check that the sampling frequency index is valid
            if (m_SamplingFrequencyIndex >= 0xD) {
                return AP4_FAILURE;
            }

            /* MPEG2 does not use all profiles */
            if (m_Id == 1 && m_ProfileObjectType == 3) {
                return AP4_FAILURE;
            }

            return AP4_SUCCESS;
        }

        // members

        // fixed part
        unsigned int m_Id;
        unsigned int m_ProtectionAbsent;
        unsigned int m_ProfileObjectType;
        unsigned int m_SamplingFrequencyIndex;
        unsigned int m_ChannelConfiguration;

        // variable part
        unsigned int m_FrameLength;
        unsigned int m_RawDataBlocks;

        // class methods
        static bool MatchFixed(unsigned char* a, unsigned char* b)
        {
            if (a[0]         ==  b[0] &&
                    a[1]         ==  b[1] &&
                    a[2]         ==  b[2] &&
                    (a[3] & 0xF0) == (b[3] & 0xF0)) {
                return true;
            } else {
                return false;
            }
        }
    };
    void genDsi(AdtsHeader& h)
    {
        uint32_t object_type = 2;
        uint32_t nIndex = h.m_SamplingFrequencyIndex;
        _dsi[0] = (object_type << 3) | (nIndex >> 1);
        _dsi[1] = uint8_t(((nIndex & 1) << 7) | (h.m_ChannelConfiguration << 3));
    }
public:
    aac_adts2asc()
    {
        memset(_dsi, 0, sizeof(_dsi));
    }
    /**
     * @brief process
     * @param[in] buf　adts数据
     * @param[in] size　adts数据大小．
     * @param[out] offset asc数据偏移
     * @return 0, success; -1 need more data
     */
    int process(uint8_t* buf, uint32_t size, uint32_t& offset)
    {
        static int const adts_header_size = 7;
        if (size < adts_header_size || buf[0] != 0xff)
            return -1;
        AdtsHeader a(buf);
        genDsi(a);
        offset = adts_header_size;
        return 0;
    }
    /**
     * @brief 获取DSI信息．
     * @param[out] out
     * @param[in out] size
     * @return 0,success; -1,not enougth memory;-2, not init
     */
    int32_t getDsi(uint8_t* out, uint32_t& size)
    {

        if (NULL == out || size < sizeof(_dsi))
        {
            size = sizeof(_dsi);
            return -1;
        }
        if (0 == _dsi[0])
            return -2;
        memcpy(out, _dsi, sizeof(_dsi));
        size = sizeof(_dsi);
        return 0;
    }
};

#endif // SRSFLVCODEC_HPP
