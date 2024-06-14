#ifndef IMULITIMEDIA_H_INCLUDED
#define IMULITIMEDIA_H_INCLUDED

#include <stdint.h>
#include "interface.h"
#include "url.h"
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

// {8E669CB7-4396-4576-A58D-12AA57F13D3C}
static const CLSID CLSID_CFilterGraph =
{ 0x8e669cb7, 0x4396, 0x4576, { 0xa5, 0x8d, 0x12, 0xaa, 0x57, 0xf1, 0x3d, 0x3c } };

const char FILTER_TYPE_SOURCE[]    = "source";
const char FILTER_TYPE_RENDER[]    = "render";
const char FILTER_TYPE_TRANSFORM[] = "transform";

const char STREAM_BITRATE_KEY[] = "bitrate";
const char STREAM_LENGTH_KEY[] = "length";
const char STREAM_START_TIME_KEY[] = "start_time";
const char STREAM_EXTRADATA_KEY[] = "extradata";
const char STREAM_GLOBALHEADER_KEY[] = "global_header";

const char FILTER_TIMEOUT_KEY[] = "timeout";

const char AUDIO_FRAME_SIZE_KEY[] = "frame_size";
const char AUDIO_MEDIA_TYPE_KEY[] = "sample_fmt";
const char AUDIO_CHANNELS_KEY[] = "channels";
const char AUDIO_SAMPLE_RATE_KEY[] = "sample_rate";
const char AUDIO_CHANNEL_LAYOUT_KEY[] = "channel_layout";

const char VIDEO_DURATION_KEY[] = "duration";
const char VIDEO_FPS_KEY[] = "fps";
const char VIDEO_MEDIA_TYPE_KEY[] = "pix_fmt";
const char VIDEO_WIDTH_KEY[] = "width";
const char VIDEO_STRIDE_KEY[] = "stride";
const char VIDEO_HEIGHT_KEY[] = "height";
const char VIDEO_RATIOX_KEY[] = "ratioX";
const char VIDEO_RATIOY_KEY[] = "ratioY";

const uint32_t MEDIA_FRAME_FLAG_SYNCPOINT     = 1;                                  //KeyFrame
const uint32_t MEDIA_FRAME_FLAG_CORRUPT       = MEDIA_FRAME_FLAG_SYNCPOINT<<1;      //data corrupt
const uint32_t MEDIA_FRAME_FLAG_NEWSEGMENT    = MEDIA_FRAME_FLAG_CORRUPT<<1;        //New segment
const uint32_t MEDIA_FRAME_FLAG_MEDIA_CHANGE  = MEDIA_FRAME_FLAG_NEWSEGMENT<<1;     //Media change
//const uint32_t MEDIA_FRAME_FLAG_LIVE          = MEDIA_FRAME_FLAG_NEWSEGMENT<<1;     //Media change
const int64_t MEDIA_FRAME_NONE_TIMESTAMP  = 0x8000000000000001;                         //None Timestamp

struct MEDIA_FRAME_INFO
{
 	int64_t pts;
	int64_t dts;
	int64_t duration;
	uint32_t flag;
	union
	{
        int samples;
        int stride;
	};
	bool segment;
	uint32_t tag;
	void* pExt;
};

#define MEDIAFRAME_COPY_INFO 1
#define MEDIAFRAME_COPY_DATA 2

INTERFACE(IMediaFrame)
{
	MEDIA_FRAME_INFO info;
	STDMETHOD(Alloc)(
		uint32_t size
		) PURE;
	STDMETHOD_(uint8_t*,GetBuf)(
		uint32_t* pSize = NULL
		) PURE;
    STDMETHOD(SetBuf)(
        uint8_t* pBuf,
        uint32_t lenBuf
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

const uint32_t MT_PROP_INTRA_ONLY = 1;
const uint32_t MT_PROP_LOSSY      = MT_PROP_INTRA_ONLY << 1;
const uint32_t MT_PROP_LOSSLESS   = MT_PROP_LOSSY << 1;
const uint32_t STREAM_NONE_INDEX  = 0xffffffff;

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
    STDMETHOD_(unsigned int,GetProps)(
        ) PURE;
	STDMETHOD_(const char*,GetMajorName)(
		) PURE;
	STDMETHOD_(const char*,GetSubName)(
		) PURE;
	STDMETHOD_(const char*,GetSubLongName)(
		) PURE;
	STDMETHOD_(bool,IsCompress)(
        ) PURE;
	STDMETHOD(Compare)(
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
        int* extra_size = NULL
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
};

const uint32_t PIN_INVALID_INDEX = 0xffffffff;
class IFilter;
INTERFACE(IPin)
{
    STDMETHOD_(IFilter*,GetFilter)(
        ) PURE;
    STDMETHOD(GetMediaType)(
        IMediaType* pMT
        ) PURE;
    STDMETHOD_(IMediaType*,GetMediaType)(
        ) PURE;
    STDMETHOD_(uint32_t,GetIndex)(
        ) PURE;
    STDMETHOD(Write)(
        IMediaFrame* pFrame
        ) PURE;
    STDMETHOD(SetTag)(
        void* pTag
        ) PURE;
    STDMETHOD_(void*,GetTag)(
        ) PURE;
};

const HRESULT S_MORE_FRAME = S_FALSE + 1;
const HRESULT S_FULL_FRAME = S_MORE_FRAME + 1;
const HRESULT S_STREAM_EOF = S_FULL_FRAME + 1;

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
};

INTERFACE_(IOutputPin,IPin)
{
    STDMETHOD(Connect)(
        IInputPin* pPin,
        IMediaType* pMT = NULL
        ) PURE;
    STDMETHOD(Disconnect)(
        IInputPin* pPin = NULL
        ) PURE;
    STDMETHOD_(IInputPin*,GetConnection)(
        ) PURE;
    STDMETHOD_(void,NewSegment)(
        ) PURE;
    STDMETHOD(AllocFrame)(
        IMediaFrame** ppFrame
        ) PURE;
};

class IFilter;
struct IFilterEvent
{
    enum EventType
    {
        Open = 0,
        Process,
        Close,
        FlushBegin,
        FlushEnd,
        End
    };
    STDMETHOD(OnEvent)(
        EventType type,
        HRESULT hr,
        IFilter* pFilter = NULL,
        IInputPin* pPinIn = NULL,
        IOutputPin* pPinOut = NULL,
        IMediaFrame* pFrame = NULL,
        void* pTag = NULL
        ) PURE;
};

typedef HRESULT FILTER_CHECK_MEDIATYPE_FUNC(IMediaType* pMtIn,IMediaType* pMtOut);
const uint32_t FILTER_FLAG_LIVE = 0x00000001;

INTERFACE(IFilter)
{
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
    STDMETHOD(Open)(
        ) PURE;
    STDMETHOD(Close)(
        ) PURE;
    STDMETHOD(SetTag)(
        void* pTag
        ) PURE;
    STDMETHOD_(void*,GetTag)(
        ) PURE;
    STDMETHOD_(double,GetExpend)(
        ) PURE;
    STDMETHOD(OnGetMediaType)(
        IInputPin* pPin,
        IMediaType* pMT
        ) PURE;
    STDMETHOD(OnGetMediaType)(
        IOutputPin* pPin,
        IMediaType* pMT
        ) PURE;
    STDMETHOD(OnSetMediaType)(
        IInputPin* pPin,
        IMediaType* pMT
        ) PURE;
    STDMETHOD(OnSetMediaType)(
        IOutputPin* pPin,
        IMediaType* pMT
        ) PURE;
    STDMETHOD(OnWriteFrame)(
        IInputPin* pPin,
        IMediaFrame* pFrame
        ) PURE;
    STDMETHOD(OnNotify)(
        IFilterEvent::EventType type,
        HRESULT hr,
        IInputPin* pPinIn,
        IOutputPin* pPinOut,
        IMediaFrame* pFrame
        ) PURE;
};

INTERFACE(IDemuxer)
{
    STDMETHOD(Load)(
        const char* pUrl = NULL
        ) PURE;
    STDMETHOD_(IOutputPin*,CreatePin)(
        IMediaType* pMT
        ) PURE;
    STDMETHOD_(void,SetStartTime)(
        const int64_t& time = MEDIA_FRAME_NONE_TIMESTAMP
        ) PURE;
    STDMETHOD_(int64_t,GetTime)(
        ) PURE;
    STDMETHOD(Process)(
        ) PURE;
    STDMETHOD_(void,Clear)(
        ) PURE;
    STDMETHOD_(bool,IsEOF)(
        ) PURE;
    STDMETHOD_(void,NewSegment)(
        ) PURE;
};

INTERFACE(IMuxer)
{
    STDMETHOD(Load)(
        const char* pUrl = NULL
        ) PURE;
    STDMETHOD_(IInputPin*,CreatePin)(
        IMediaType* pMT
        ) PURE;
    STDMETHOD_(void,Clear)(
        ) PURE;
};

typedef HRESULT URL_SUPPORT_QUERY_FUNC(const char* pProtocol,const char* pFormat);

struct filter_graph_status
{
    bool isLive;
    int64_t clockStart;
    int64_t clockTime;
    int64_t timeStart;
    int64_t timeInput;
    int64_t timeOutput;
};

INTERFACE(IFilterGraph)
{
    STDMETHOD(LoadSource)(
        const char* pUrl,
        IFilter** ppFilter,
        const char* pName = NULL
        ) PURE;
    STDMETHOD(LoadRender)(
        const char* pUrl,
        IFilter** ppFilter,
        const char* pName = NULL
        ) PURE;
    STDMETHOD(ConnectPin)(
        IOutputPin* pPinOut,
        IInputPin* pPinIn,
        IMediaType* pMT = NULL
        ) PURE;
    STDMETHOD(ConnectPin)(
        IOutputPin* pPin,
        IMediaType* pMT,
        const char* pName,
        IFilter** ppFilter
        ) PURE;
    STDMETHOD(ConnectPin)(
        IInputPin* pPin,
        IMediaType* pMT,
        const char* pName,
        IFilter** ppFilter
        ) PURE;
    STDMETHOD(Remove)(
        IFilter* pFilter
        ) PURE;
    STDMETHOD(Play)(
        bool isWait = false
        ) PURE;
    STDMETHOD(Stop)(
        bool isWait = true
        ) PURE;
    STDMETHOD_(bool,IsRunning)(
        ) PURE;
    STDMETHOD(SetEvent)(
        IFilterEvent* pEvent,
        void* pTag = NULL
        ) PURE;
    STDMETHOD_(const filter_graph_status&,GetStatus)(
        ) PURE;
    STDMETHOD_(bool,IsExit)(
        ) PURE;
    STDMETHOD(Clear)(
        ) PURE;
};

struct IFilterGraphEvent
{
    STDMETHOD(Notify)(
        IFilterEvent::EventType type,
        HRESULT hr,
        IFilter* pFilter = NULL,
        IInputPin* pPinIn = NULL,
        IOutputPin* pPinOut = NULL,
        IMediaFrame* pFrame = NULL
        ) PURE;
    STDMETHOD_(bool,IsExit)(
        ) PURE;
    STDMETHOD_(const filter_graph_status&,GetStatus)(
        ) PURE;
};


#endif // IMULITIMEDIA_H_INCLUDED
