#ifndef IMULITIMEDIA_H_INCLUDED
#define IMULITIMEDIA_H_INCLUDED

#include <stdint.h>
#include "interface.h"
#include "iStream.h"
#include "MediaID.h"
#include "iProfile.h"

// {7353F76C-A1A1-4d4f-988A-86C9C9311B02}
static const CLSID CLSID_CMediaType =
{ 0x7353f76c, 0xa1a1, 0x4d4f, { 0x98, 0x8a, 0x86, 0xc9, 0xc9, 0x31, 0x1b, 0x2 } };

// {6CB751FA-6C9B-4F69-911C-7DC15E47DECE}
static const CLSID CLSID_CMediaFrame =
{ 0x6cb751fa, 0x6c9b, 0x4f69, { 0x91, 0x1c, 0x7d, 0xc1, 0x5e, 0x47, 0xde, 0xce } };

// {17FD624D-00EC-4b7b-8CB4-95BC003212EF}
static const CLSID CLSID_CMediaFrameAllocate =
{ 0x17fd624d, 0xec, 0x4b7b, { 0x8c, 0xb4, 0x95, 0xbc, 0x0, 0x32, 0x12, 0xef } };

// {75D6994C-2EB9-4e3b-BE22-E6F475C00B00}
static const CLSID CLSID_CInputPin =
{ 0x75d6994c, 0x2eb9, 0x4e3b, { 0xbe, 0x22, 0xe6, 0xf4, 0x75, 0xc0, 0xb, 0x0 } };

// {19638EB5-308D-42e7-9431-CFF592E2C3A0}
static const CLSID CLSID_COutputPin =
{ 0x19638eb5, 0x308d, 0x42e7, { 0x94, 0x31, 0xcf, 0xf5, 0x92, 0xe2, 0xc3, 0xa0 } };

// {7266326C-A0B0-4d4f-9985-BFA690BCF68C}
static const CLSID CLSID_CFFmpegDemuxer =
{ 0x7266326c, 0xa0b0, 0x4d4f, { 0x99, 0x85, 0xbf, 0xa6, 0x90, 0xbc, 0xf6, 0x8c } };

// {5A2DD1CB-ECE9-40d9-B97C-ACC527505A75}
static const CLSID CLSID_CFFmpegMuxer =
{ 0x5a2dd1cb, 0xece9, 0x40d9, { 0xb9, 0x7c, 0xac, 0xc5, 0x27, 0x50, 0x5a, 0x75 } };

// {1870A349-C0EA-4cc4-A0BB-066C9E965793}
static const CLSID CLSID_CFFmpegAudioDecoder =
{ 0x1870a349, 0xc0ea, 0x4cc4, { 0xa0, 0xbb, 0x6, 0x6c, 0x9e, 0x96, 0x57, 0x93 } };

// {C48CF9FF-9614-47ca-95AF-6B41C628AAB2}
static const CLSID CLSID_CFFmpegAudioEncoder =
{ 0xc48cf9ff, 0x9614, 0x47ca, { 0x95, 0xaf, 0x6b, 0x41, 0xc6, 0x28, 0xaa, 0xb2 } };

// {F74BF10E-9D35-47ba-BB4B-9C203C59940A}
static const CLSID CLSID_CFFmpegVideoDecoder =
{ 0xf74bf10e, 0x9d35, 0x47ba, { 0xbb, 0x4b, 0x9c, 0x20, 0x3c, 0x59, 0x94, 0xa } };

// {0E3F53FE-0DB0-4023-B61B-EDA3834818B6}
static const CLSID CLSID_CFFmpegVideoEncoder =
{ 0xe3f53fe, 0xdb0, 0x4023, { 0xb6, 0x1b, 0xed, 0xa3, 0x83, 0x48, 0x18, 0xb6 } };

// {5CFC4210-2A7F-4343-9AAE-FAB5787D54B3}
static const CLSID CLSID_CFFmpegAudioResample =
{ 0x5cfc4210, 0x2a7f, 0x4343, { 0x9a, 0xae, 0xfa, 0xb5, 0x78, 0x7d, 0x54, 0xb3 } };

// {AA5D0004-9C3C-4f75-B082-78A94F0E4AC9}
static const CLSID CLSID_CFFmpegVideoScale =
{ 0xaa5d0004, 0x9c3c, 0x4f75, { 0xb0, 0x82, 0x78, 0xa9, 0x4f, 0xe, 0x4a, 0xc9 } };

// {CFBE69C4-2B04-4ab8-AD8F-AFCA62C90B5E}
static const CLSID CLSID_CIntelVideoDecoder =
{ 0xcfbe69c4, 0x2b04, 0x4ab8, { 0xad, 0x8f, 0xaf, 0xca, 0x62, 0xc9, 0xb, 0x5e } };

// {FF89F49B-6CBB-403c-B592-80E148049225}
static const CLSID CLSID_CIntelVideoEncoder =
{ 0xff89f49b, 0x6cbb, 0x403c, { 0xb5, 0x92, 0x80, 0xe1, 0x48, 0x4, 0x92, 0x25 } };

// {EB4539B9-829B-43d7-8D2E-45A93E401C6F}
static const CLSID CLSID_CNvidiaVideoDecoder =
{ 0xeb4539b9, 0x829b, 0x43d7, { 0x8d, 0x2e, 0x45, 0xa9, 0x3e, 0x40, 0x1c, 0x6f } };

// {8E0533AC-5AFC-4b42-AD94-5D7A4B8B55C7}
static const CLSID CLSID_CNvidiaVideoEncoder =
{ 0x8e0533ac, 0x5afc, 0x4b42, { 0xad, 0x94, 0x5d, 0x7a, 0x4b, 0x8b, 0x55, 0xc7 } };

// {BD93F85B-FB6A-4D7D-A6EC-3A10215D95DE}
static const CLSID CLSID_CSdlRender =
{ 0xbd93f85b, 0xfb6a, 0x4d7d, { 0xa6, 0xec, 0x3a, 0x10, 0x21, 0x5d, 0x95, 0xde } };

// {B31E4758-5478-448F-9946-D8EFEABFB9EA}
static const CLSID CLSID_CFFmpegOSD =
{ 0xb31e4758, 0x5478, 0x448f, { 0x99, 0x46, 0xd8, 0xef, 0xea, 0xbf, 0xb9, 0xea } };

// {D4E424A8-A62C-4AE1-B7A0-E011D1F2534E}
static const CLSID CLSID_CCapture =
{ 0xd4e424a8, 0xa62c, 0x4ae1, { 0xb7, 0xa0, 0xe0, 0x11, 0xd1, 0xf2, 0x53, 0x4e } };

// {C7F85B3A-18C6-4524-B84E-94911C5D8208}
static const CLSID CLSID_CPublishRender =
{ 0xc7f85b3a, 0x18c6, 0x4524, { 0xb8, 0x4e, 0x94, 0x91, 0x1c, 0x5d, 0x82, 0x8 } };

// {8E669CB7-4396-4576-A58D-12AA57F13D3C}
static const CLSID CLSID_CFilterGraph =
{ 0x8e669cb7, 0x4396, 0x4576, { 0xa5, 0x8d, 0x12, 0xaa, 0x57, 0xf1, 0x3d, 0x3c } };

// {D9569F03-A068-43AC-8639-2FC09D0798BA}
static const CLSID CLSID_CGosunDemuxer =
{ 0xd9569f03, 0xa068, 0x43ac, { 0x86, 0x39, 0x2f, 0xc0, 0x9d, 0x7, 0x98, 0xba } };

// {F6358134-CCB2-41E9-A73E-8743B36547BD}
static const CLSID CLSID_CGosunMuxer =
{ 0xf6358134, 0xccb2, 0x41e9, { 0xa7, 0x3e, 0x87, 0x43, 0xb3, 0x65, 0x47, 0xbd } };

// {5AAD7A65-0027-4587-87BE-BA795C039A48}
static const CLSID CLSID_CGosunSession =
{ 0x5aad7a65, 0x27, 0x4587, { 0x87, 0xbe, 0xba, 0x79, 0x5c, 0x3, 0x9a, 0x48 } };

// {008C621C-91FE-4419-9751-B775ED6B7376}
static const CLSID CLSID_CFlvDemuxer =
{ 0x8c621c, 0x91fe, 0x4419, { 0x97, 0x51, 0xb7, 0x75, 0xed, 0x6b, 0x73, 0x76 } };

// {B8633660-7319-4ADC-ABBA-CA5724D530B0}
static const CLSID CLSID_CFlvMuxer =
{ 0xb8633660, 0x7319, 0x4adc, { 0xab, 0xba, 0xca, 0x57, 0x24, 0xd5, 0x30, 0xb0 } };

// {F296CA1F-1E40-48FB-815B-CCFD007BACE5}
static const CLSID CLSID_CRtmpSession =
{ 0xf296ca1f, 0x1e40, 0x48fb, { 0x81, 0x5b, 0xcc, 0xfd, 0x0, 0x7b, 0xac, 0xe5 } };

// {5A2FEECB-8A01-48EC-AA6D-0972F7FE1EA6}
static const CLSID CLSID_CTsDemuxer =
{ 0x5a2feecb, 0x8a01, 0x48ec, { 0xaa, 0x6d, 0x9, 0x72, 0xf7, 0xfe, 0x1e, 0xa6 } };

// {389E85B9-F984-44BF-B5C4-2F11C956D4A8}
static const CLSID CLSID_CTsMuxer =
{ 0x389e85b9, 0xf984, 0x44bf, { 0xb5, 0xc4, 0x2f, 0x11, 0xc9, 0x56, 0xd4, 0xa8 } };

// {CB629B6A-D752-49A6-995E-ABCB4E24FF06}
static const CLSID CLSID_CHttpSession =
{ 0xcb629b6a, 0xd752, 0x49a6, { 0x99, 0x5e, 0xab, 0xcb, 0x4e, 0x24, 0xff, 0x6 } };

// {8B8B953B-1E75-4DFC-B2E7-717264E75C1A}
static const CLSID CLSID_CFileSession =
{ 0x8b8b953b, 0x1e75, 0x4dfc, { 0xb2, 0xe7, 0x71, 0x72, 0x64, 0xe7, 0x5c, 0x1a } };

// {6B447D0C-A3B6-4AF6-8E10-F59AA05386FC}
static const CLSID CLSID_CApiSession =
{ 0x6b447d0c, 0xa3b6, 0x4af6, { 0x8e, 0x10, 0xf5, 0x9a, 0xa0, 0x53, 0x86, 0xfc } };

// {6B447D0C-A3B6-4AF6-8E10-F59AA05386FC}
static const CLSID CLSID_CWebRtcSession =
{ 0x6b447d0c, 0xa3b6, 0x4af6, { 0x8e, 0x10, 0xf5, 0x9a, 0xa0, 0x53, 0x86, 0xfc } };

const char FILTER_TYPE_NAME[] = "filter";

enum FilterType
{
    FT_None      = 0,
    FT_Source    = 1,
    FT_Render    = 2,
    FT_Session   = FT_Source | FT_Render,
    FT_Transform = 4
};

const char PROTOCOL_PUBLISH_NAME[] = "publish";

const char FILTER_TYPE_TRANSFORM[] = "transform";

const char STREAM_BITRATE_KEY[] = "bitrate";
const char STREAM_LENGTH_KEY[] = "length";
const char STREAM_START_TIME_KEY[] = "start_time";
const char STREAM_START_TIME_CLOCK_VALUE[] = "clock";
const char STREAM_EXTRADATA_KEY[] = "extradata";
const char STREAM_GLOBALHEADER_KEY[] = "global_header";

const char FILTER_TIMEOUT_KEY[] = "timeout";
const char FILTER_HEADER_KEY[] = "header";

const char AUDIO_FRAME_SIZE_KEY[] = "frame_size";
const char AUDIO_MEDIA_TYPE_KEY[] = "sample_fmt";
const char AUDIO_CHANNELS_KEY[] = "channels";
const char AUDIO_SAMPLE_RATE_KEY[] = "sample_rate";
const char AUDIO_CHANNEL_LAYOUT_KEY[] = "channel_layout";

const char VIDEO_FPS_KEY[] = "fps";
const char VIDEO_MEDIA_TYPE_KEY[] = "pix_fmt";
const char VIDEO_WIDTH_KEY[] = "width";
const char VIDEO_STRIDE_KEY[] = "stride";
const char VIDEO_HEIGHT_KEY[] = "height";
const char VIDEO_RATIOX_KEY[] = "ratioX";
const char VIDEO_RATIOY_KEY[] = "ratioY";

const uint8_t MEDIA_FRAME_FLAG_SYNCPOINT     = 1;                                  //KeyFrame
const uint8_t MEDIA_FRAME_FLAG_NEWSEGMENT    = MEDIA_FRAME_FLAG_SYNCPOINT<<1;        //New segment
const uint8_t MEDIA_FRAME_FLAG_SEGMENT       = MEDIA_FRAME_FLAG_NEWSEGMENT<<1;            //Media change
const uint8_t MEDIA_FRAME_FLAG_EOF           = MEDIA_FRAME_FLAG_SEGMENT<<1;     //EOF
const uint8_t MEDIA_FRAME_FLAG_CORRUPT       = MEDIA_FRAME_FLAG_EOF<<1;      //data corrupt
const uint8_t MEDIA_FRAME_FLAG_MEDIA_CHANGE  = MEDIA_FRAME_FLAG_CORRUPT<<1;            //Media change
const int64_t MEDIA_FRAME_NONE_TIMESTAMP  = 0x8000000000000001;                         //None Timestamp

const uint32_t ET_Filter_Build  = ET_Session_Pull + 1;
const uint32_t ET_Filter_Render = ET_Filter_Build + 1;
const uint32_t ET_Filter_Buffer = ET_Filter_Render + 1;

const uint32_t ET_Publish_Add   = ET_Filter_Buffer + 1;
const uint32_t ET_Publish_Del   = ET_Publish_Add + 1;

const uint32_t ET_Api_Push   = ET_Publish_Del + 1;
const uint32_t ET_Api_Pull   = ET_Api_Push + 1;

struct MEDIA_FRAME_INFO
{
 	int64_t pts;
	int64_t dts;
	int64_t duration;
	uint8_t flag;
	union
	{
        int32_t samples;
        int32_t stride;
        uint32_t msg;
	};
	bool segment;
	uint32_t tag;
	void* pExt;
};

#define MEDIAFRAME_COPY_INFO 1
#define MEDIAFRAME_COPY_DATA 2

const uint32_t MAX_MEDIA_FRAME_BUF_COUNT = 0xffffffff;
INTERFACE(IMediaFrame)
{
    struct serialize_param
    {
        uint32_t id;
        int64_t base;
    };
    struct buf
    {
        uint8_t* data;
        size_t size;
    };
	MEDIA_FRAME_INFO info;
	STDMETHOD(SetBuf)(
        uint32_t index,
		uint32_t size,
		const void* data = NULL,
		uint8_t flag = 0
		) PURE;
	STDMETHOD(SetBufs)(
		const buf* buf,
		uint32_t size,
        uint32_t index = 0,
		uint8_t flag = 0
		) PURE;
	STDMETHOD_(const buf*,GetBuf)(
        uint32_t index = 0,
        uint32_t* count = NULL,
        uint32_t* size = NULL
		) PURE;
	STDMETHOD(CopyTo)(
		IMediaFrame* pFrame,
		uint32_t flag = MEDIAFRAME_COPY_INFO|MEDIAFRAME_COPY_DATA
		) PURE;
	STDMETHOD(CopyFrom)(
		IMediaFrame* pFrame,
		uint32_t flag = MEDIAFRAME_COPY_INFO|MEDIAFRAME_COPY_DATA
		) PURE;
	STDMETHOD_(void,Clear)(
		) PURE;
};

INTERFACE(IMediaFrameAllocate)
{
	STDMETHOD(Alloc)(
		IMediaFrame** ppFrame
		) PURE;
	STDMETHOD_(void,Clear)(
		) PURE;
};

const uint32_t MT_PROP_LOSSY     = 1 << 1;
const uint32_t MT_PROP_LOSSLESS  = 1 << 2;
const uint32_t MT_PROP_REORDER   = 1 << 3;

const uint32_t STREAM_NONE_INDEX = 0xffffffff;

INTERFACE(IMediaType)
{
    STDMETHOD(SetMajor)(
        MediaMajorType major
        ) PURE;
	STDMETHOD(SetMajor)(
		const char* pName
		) PURE;
    STDMETHOD_(MediaMajorType,GetMajor)(
        ) PURE;
	STDMETHOD(SetSub)(
		MediaSubType type
		) PURE;
	STDMETHOD(SetSub)(
		const char* pName
		) PURE;
	STDMETHOD_(MediaSubType,GetSub)(
		) PURE;
    STDMETHOD_(uint32_t,GetProps)(
        ) PURE;
	STDMETHOD_(const char*,GetMajorName)(
		) PURE;
	STDMETHOD_(const char*,GetSubName)(
		) PURE;
	STDMETHOD_(const char*,GetSubLongName)(
		) PURE;
	STDMETHOD_(bool,IsCompress)(
        ) PURE;
	STDMETHOD_(uint32_t,Compare)(
		IMediaType* pMT
		) PURE;
	STDMETHOD(CopyFrom)(
		IMediaType* pMT,
		uint32_t flag = 0
		) PURE;
	STDMETHOD(CopyTo)(
		IMediaType* pMT,
		uint32_t flag = 0
		) PURE;
    STDMETHOD(Clear)(
        ) PURE;
    STDMETHOD_(uint32_t,GetFourcc)(
        uint32_t flag = 0
        ) PURE;
    STDMETHOD(SetFourcc)(
        uint32_t fourcc
        ) PURE;
    STDMETHOD(GetStreamInfo)(
        int64_t* start_time = NULL,
        int64_t* length = NULL,
        int64_t* bitrate = NULL,
        bool* global_header = NULL,
        uint8_t** extra_data = NULL,
        int* extra_size = NULL
        ) PURE;
    STDMETHOD(SetStreamInfo)(
        int64_t* start_time = NULL,
        int64_t* length = NULL,
        int64_t* bitrate = NULL,
        bool* global_header = NULL,
        uint8_t* extra_data = NULL,
        int* extra_size = NULL,
        uint8_t** extra_data_out = NULL
        ) PURE;
    STDMETHOD(GetVideoInfo)(
        VideoMediaType* pix_fmt = NULL,
        int* width = NULL,
        int* height = NULL,
        int* ratioX = NULL,
        int* ratioY = NULL,
        int64_t* duration = NULL
        ) PURE;
    STDMETHOD(SetVideoInfo)(
        VideoMediaType* pix_fmt = NULL,
        int* width = NULL,
        int* height = NULL,
        int* ratioX = NULL,
        int* ratioY = NULL,
        int64_t* duration = NULL
        ) PURE;
    STDMETHOD(GetAudioInfo)(
        AudioMediaType* sample_fmt = NULL,
        uint64_t* channel_layout = NULL,
        int* channels = NULL,
        int* sample_rate = NULL,
        int* frame_size = NULL
        ) PURE;
    STDMETHOD(SetAudioInfo)(
        AudioMediaType* sample_fmt = NULL,
        uint64_t* channel_layout = NULL,
        int* channels = NULL,
        int* sample_rate = NULL,
        int* frame_size = NULL
        ) PURE;
    STDMETHOD(FrameAlloc)(
        IMediaFrame* frm
        ) PURE;
    STDMETHOD(FrameToArray)(
        uint8_t** dst_data,
        int* dst_linesize,
        IMediaFrame* src
        ) PURE;
    STDMETHOD(ArrayToFrame)(
        IMediaFrame* dst,
        const uint8_t** src_data,
        const int* src_linesize
        ) PURE;
    STDMETHOD(SetFrame)(
        IMediaFrame* pFrame
        ) PURE;
    STDMETHOD_(IMediaFrame*,GetFrame)(
        ) PURE;
};

const uint32_t PIN_INVALID_INDEX = 0xffffffff;
class IFilter;
INTERFACE(IPin)
{
    STDMETHOD_(IFilter*,GetFilter)(
        ) PURE;
    STDMETHOD_(IMediaType*,GetMediaType)(
        ) PURE;
    STDMETHOD(SetMediaType)(
        IMediaType* pMT
        ) PURE;
    STDMETHOD_(uint32_t,GetIndex)(
        ) PURE;
    STDMETHOD_(void,SetID)(
        uint32_t id
        ) PURE;
    STDMETHOD_(uint32_t,GetID)(
        ) PURE;
    STDMETHOD(Send)(
        uint32_t cmd,
        bool down = true,
        bool first = false
        ) PURE;
    STDMETHOD_(uint32_t,Recv)(
        ) PURE;
    STDMETHOD_(void,Set)(
        uint32_t cmd
        ) PURE;
    STDMETHOD(Write)(
        IMediaFrame* pFrame
        ) PURE;
    STDMETHOD_(void,SetTag)(
        void* pTag
        ) PURE;
    STDMETHOD_(void*,GetTag)(
        ) PURE;
    STDMETHOD_(void,SetObj)(
        Interface* pObj
        ) PURE;
    STDMETHOD_(Interface*,GetObj)(
        ) PURE;
    STDMETHOD(SetFlag)(
        uint8_t flag
        ) PURE;
};

class IOutputPin;
INTERFACE_(IInputPin,IPin)
{
    STDMETHOD(Connect)(
        IOutputPin* pPin,
        IMediaType* pMT
        ) PURE;
    STDMETHOD_(void,Disconnect)(
        ) PURE;
    STDMETHOD_(IOutputPin*,GetConnection)(
        ) PURE;
    STDMETHOD_(IInputPin*,Next)(
        ) PURE;
    STDMETHOD(Pop)(
        IMediaFrame** ppFrame = NULL
        ) PURE;
    STDMETHOD_(int64_t,GetBufLen)(
        ) PURE;
    STDMETHOD_(bool,IsEnd)(
        ) PURE;
};

INTERFACE_(IOutputPin,IPin)
{
    STDMETHOD(Connect)(
        IInputPin* pPin,
        IMediaType* pMT = NULL
        ) PURE;
    STDMETHOD_(void,Disconnect)(
        IInputPin* pPin = NULL
        ) PURE;
    STDMETHOD_(IInputPin*,GetConnection)(
        ) PURE;
    STDMETHOD(AllocFrame)(
        IMediaFrame** ppFrame
        ) PURE;
    STDMETHOD(SetClock)(
        bool enable
        ) PURE;
};

typedef HRESULT FILTER_QUERY_FUNC(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);

const uint32_t Filter_CMD_None = 0;

INTERFACE(IFilter)
{
    static const uint32_t FLAG_LIVE = IStream::SEEK_FLAG<<1;
    enum Status
    {
        S_Stop = Filter_CMD_None + 1,
        S_Pause,
        S_Play,
        S_NB
    };
    enum Command
    {
        C_Flush = S_NB,
        C_Profile,
        C_Accept,
        C_Enable,
        C_Disable,
        C_NB
    };

    STDMETHOD_(FilterType,GetType)(
        ) PURE;
    STDMETHOD(SetName)(
        const char* pName
        ) PURE;
    STDMETHOD_(const char*,GetName)(
        ) PURE;
    STDMETHOD_(uint32_t,GetFlag)(
        ) PURE;
    STDMETHOD_(uint32_t,GetInputPinCount)(
        ) PURE;
    STDMETHOD_(IInputPin*,GetInputPin)(
        uint32_t index
        ) PURE;
    STDMETHOD_(uint32_t,GetOutputPinCount)(
        ) PURE;
    STDMETHOD_(IOutputPin*,GetOutputPin)(
        uint32_t index
        ) PURE;
    STDMETHOD_(IInputPin*,CreateInputPin)(
        IMediaType* pMT
        ) PURE;
    STDMETHOD_(IOutputPin*,CreateOutputPin)(
        IMediaType* pMT
        ) PURE;
    STDMETHOD(Notify)(
        uint32_t cmd
        ) PURE;
    STDMETHOD_(uint32_t,GetStatus)(
        ) PURE;
    STDMETHOD_(void,SetTag)(
        void* pTag
        ) PURE;
    STDMETHOD_(void*,GetTag)(
        ) PURE;
    STDMETHOD_(double,GetExpend)(
        ) PURE;
    STDMETHOD(OnConnect)(
        IInputPin* pPin,
        IOutputPin* pPinOut,
        IMediaType* pMT
        ) PURE;
    STDMETHOD(OnConnect)(
        IOutputPin* pPin,
        IInputPin* pPinIn,
        IMediaType* pMT
        ) PURE;
    STDMETHOD(OnWriteFrame)(
        IInputPin* pPin,
        IMediaFrame* pFrame
        ) PURE;
};

INTERFACE(ILoad)
{
    static const uint8_t LOAD_FLAG_STREAM_ID = 1;
    STDMETHOD(Load)(
        const char* pUrl,
        uint32_t mode = 0,
        uint8_t flag = 0
        ) PURE;
    STDMETHOD_(const char*,GetName)(
        ) PURE;
};

INTERFACE(ISource)
{
    STDMETHOD_(void,SetStartTime)(
        const int64_t& time = MEDIA_FRAME_NONE_TIMESTAMP
        ) PURE;
    STDMETHOD_(int64_t,GetTime)(
        ) PURE;
    STDMETHOD_(bool,IsEOF)(
        ) PURE;
    STDMETHOD_(void,NewSegment)(
        ) PURE;
};

INTERFACE(IFormat)
{
    STDMETHOD_(IInputPin*,CreateInputPin)(
        IMediaType* pMT
        ) PURE;
    STDMETHOD_(IOutputPin*,CreateOutputPin)(
        IMediaType* pMT
        ) PURE;
};

INTERFACE(IFilterGraph)
{
    struct Status
    {
        bool isLive;
        IFilter::Status status;
        int64_t clockStart;
        int64_t clockTime;
        int64_t timeStart;
        int64_t timeInput;
        int64_t timeOutput;
    };
    STDMETHOD(Create)(
        FilterType type,
        const char* pUrl,
        IFilter** ppFilter,
        const char* pFormat = NULL,
        const char* pName = NULL
        ) PURE;
    STDMETHOD(ConnectPin)(
        IOutputPin* pPinOut,
        IInputPin* pPinIn,
        IMediaType* pMT = NULL
        ) PURE;
    STDMETHOD(ConnectPin)(
        IOutputPin* pPin,
        IFilter** ppFilter = NULL,
        const char* pName = NULL
        ) PURE;
    STDMETHOD(ConnectPin)(
        IInputPin* pPin,
        IFilter** ppFilter = NULL,
        const char* pName = NULL
        ) PURE;
    STDMETHOD(Append)(
        IFilter* pFilter
        ) PURE;
    STDMETHOD_(uint32_t,GetCount)(
        FilterType type
        ) PURE;
    STDMETHOD(Enum)(
        FilterType type,
        IIt** ppIt
        ) PURE;
    STDMETHOD_(IFilter*,Get)(
        IIt* pIt
        ) PURE;
    STDMETHOD(Notify)(
        IFilter* pFilter,
        uint32_t cmd,
        bool first = true
        ) PURE;
    STDMETHOD_(Status&,GetStatus)(
        ) PURE;
    STDMETHOD(Clear)(
        ) PURE;
};

INTERFACE(IStreamListen)
{
    STDMETHOD(Startup)(
        const ClassInfo* info,
        uint16_t port = 0
        ) PURE;
    STDMETHOD_(void,Shutdown)(
        ) PURE;
};

typedef HRESULT CREATE_LISTENER(IStreamListen** ppListen,Interface* pObj,uint16_t* pPort);

#endif // IMULITIMEDIA_H_INCLUDED
