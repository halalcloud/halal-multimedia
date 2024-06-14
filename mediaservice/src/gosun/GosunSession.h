#ifndef GOSUNSESSION_H
#define GOSUNSESSION_H
#include "stdafx.h"
enum msg_type:uint8_t
{
    mt_none = 0x00,
    mt_push, //push request
    mt_pull, //pull request
    mt_info, //info reponse
    mt_play, //play data request
    mt_packet, //frame data
    mt_tail, //tail data
    mt_slice, //slice data
    mt_pause, //pause data request
    mt_fail, //error
    mt_nb
};
#pragma pack(1)
const uint32_t PACKET_END_SLICE_INDEX = 0xffffffff;
//const uint8_t PACKET_FLAG_SEGMENT = 1;

struct msg
{
    uint32_t index;
    int64_t time;
    msg_type type;
    uint8_t flag;
    uint32_t size;
};

#pragma pack()
const char INDEX_SECTION_NAME[] = ".index";
const char SLICE_SECTION_NAME[] = ".slice";
const char SLICE_NAME_FORMAT[] = "%04u%02u%02u/%ld.gosun";
class CGosunSession;
class CStream
{
    typedef map< int64_t,string > Clip;
    typedef Clip::iterator ClipIt;
    typedef pair<Clip::key_type,Clip::mapped_type> ClipPair;
public:
    CStream(CGosunSession* pSession);
    ~CStream();
    bool Create(void* pParam);
    HRESULT Load(const char* pUrl = NULL,uint32_t mode = 0);
    HRESULT SetName(const char* pName);
    const char* GetName();
    uint32_t GetFlag();
    HRESULT Open();
    HRESULT Play();
    HRESULT Pause();
    void Close();
    HRESULT Read(IMediaFrame** ppFrame);
    HRESULT Write(IMediaFrame* pFrame);
    HRESULT CanWrite();
protected:
    HRESULT Read(IStream* pStream,IMediaFrame** ppFrame);
    HRESULT Seek();
    string GetSlice(const string& root,int64_t pos);
    string GetRoot();
    void ClipDelete(tm* time);
    void DirDelete(string path);
protected:
    CGosunSession* m_session;
    dom_ptr<IStream> m_spStream;
    dom_ptr<IStream> m_spSlice;
    dom_ptr<IMediaFrame> m_spFrameRecv;
    dom_ptr<IMediaFrame> m_spMediaInfo;
    dom_ptr<IMediaFrame> m_spFrameRead;
    CUrl m_url;
    uint32_t m_mode;
    string m_name;
    string m_root;
    string m_clip;
    string m_slice;
    bool m_is_live;
    uint32_t m_index_recv;
    uint32_t m_index_send;
    uint32_t m_index_slice;
    uint32_t m_pos_send;
    int64_t m_start_recv;
    int64_t m_start_send;
    int64_t m_time_send;

    uint32_t m_day_save;
    uint32_t m_flag;

//    uint32_t m_pos_segment
//    Index m_index;
};

class CGosunSession : public IFilter, public ILoad, public ICallback
{
    friend class CStream;
    friend class CGosunStream;
    typedef list< dom_ptr<IMediaFrame> > FrameSet;
    typedef FrameSet::iterator FrameIt;
public:
    DOM_DECLARE(CGosunSession)
    //IFilter
    STDMETHODIMP_(FilterType) GetType();
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(uint32_t) GetInputPinCount();
    STDMETHODIMP_(IInputPin*) GetInputPin(uint32_t index);
    STDMETHODIMP_(uint32_t) GetOutputPinCount();
    STDMETHODIMP_(IOutputPin*) GetOutputPin(uint32_t index);
    STDMETHODIMP_(IInputPin*) CreateInputPin(IMediaType* pMT);
    STDMETHODIMP_(IOutputPin*) CreateOutputPin(IMediaType* pMT);
    STDMETHODIMP Notify(uint32_t cmd);
    STDMETHODIMP_(uint32_t) GetStatus();
    STDMETHODIMP_(void) SetTag(void* pTag);
    STDMETHODIMP_(void*) GetTag();
    STDMETHODIMP_(double) GetExpend();
    STDMETHODIMP OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT);
    STDMETHODIMP OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    //ILoad
    STDMETHODIMP Load(const char* pUrl,uint32_t mode,uint8_t flag = 0);
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    //CGosunSession
    HRESULT SetType(FilterType type);
    HRESULT Recv();
    HRESULT Recv(msg& packet,IMediaFrame** ppFrame);
    HRESULT Send(msg_type send,IMediaFrame* pFrame = NULL);
    HRESULT Init();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
    static HRESULT CreateListener(IStreamListen** ppListen,Interface* pObj,uint16_t* pPort);
protected:
    FilterType m_type;
    IFilter::Status m_status;
    string m_name;
    void* m_pTag;
    dom_ptr<IMediaFrameAllocate> m_spAllocate;
    dom_ptr<IInputPin> m_pinIn;
    dom_ptr<IOutputPin> m_pinOut;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
    CStream m_stream;
    uint32_t m_index_send;
    msg m_packet;
    uint32_t m_timeout;
};

#endif // GOSUNSESSION_H
