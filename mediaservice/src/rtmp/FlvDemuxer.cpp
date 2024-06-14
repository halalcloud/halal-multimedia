#include "FlvDemuxer.h"
#include "SrsFlvCodec.hpp"
#include "SrsBuffer.hpp"

CFlvDemuxer::CFlvDemuxer()
:m_tag(NULL)
,m_countPin(0)
,m_status(S_Stop)
,m_isEOF(false)
,m_type(FlvHeader)
,m_first(true)
{
    //ctor
}

bool CFlvDemuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    JCHK(m_pinIn.Create(CLSID_CInputPin,(IFilter*)this),false);
    JCHK(m_spAllocate.Create(CLSID_CMediaFrameAllocate),false);
    return true;
}

bool CFlvDemuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        clear();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFlvDemuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFlvDemuxer::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CFlvDemuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFlvDemuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFlvDemuxer::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFlvDemuxer::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CFlvDemuxer::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn : NULL;
}

STDMETHODIMP_(uint32_t) CFlvDemuxer::GetOutputPinCount()
{
    return m_countPin;
}

STDMETHODIMP_(IOutputPin*) CFlvDemuxer::GetOutputPin(uint32_t index)
{
    if(index >= m_countPin)
        return NULL;
    JCHK(index < m_countPin,NULL);

    if (index == 0) {
        return m_pinsOut_video == NULL ? m_pinsOut_audio : m_pinsOut_video;
    } else if (index == 1) {
        return m_pinsOut_video == NULL ? NULL : m_pinsOut_audio;
    } else {
        return NULL;
    }
}

STDMETHODIMP_(IInputPin*) CFlvDemuxer::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CFlvDemuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFlvDemuxer::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(S_Stop == m_status)
            {
                JIF(Open());
            }
            else if(S_Stop == cmd)
            {
                JIF(Close());
            }
            if(S_Play == cmd)
            {
                m_pinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
            }
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CFlvDemuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFlvDemuxer::SetTag(void* pTag)
{
    m_tag = pTag;
}

STDMETHODIMP_(void*) CFlvDemuxer::GetTag()
{
    return m_tag;
}

STDMETHODIMP_(double) CFlvDemuxer::GetExpend()
{
    return 0;
}

STDMETHODIMP CFlvDemuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    HRESULT hr = S_OK;

    JCHK(pPin == m_pinIn.p,E_FAIL);

    if(NULL != pMT)
    {
        if(MST_FLV_TAG != pMT->GetSub())
            return E_INVALIDARG;

        JIF(CreatePinOuts(pMT));
    }
    return S_OK;
}

STDMETHODIMP CFlvDemuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return NULL == pMT || COMPARE_SAME_VALUE == pPin->GetMediaType()->Compare(pMT) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CFlvDemuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    JCHK(m_pinIn == pPin,E_INVALIDARG);
    JCHK(NULL != pFrame,E_INVALIDARG);

    HRESULT hr = S_OK;

    // 从rtmpsession过来，pFrame为完整的flv tag，需要转成裸数据
    JIF(ProcessFlvFrame(pFrame));

    return hr;
}

//IEventCallback
STDMETHODIMP CFlvDemuxer::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    HRESULT hr;

    IStream* pStream = (IStream*)source;

    // 从httpsession过来，需要自己解析flv，转成裸数据
    JIF(FlvService(pStream));

    return m_ep->Notify(type,param1,param2,param3);
}

HRESULT CFlvDemuxer::Open()
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT CFlvDemuxer::Close()
{
    HRESULT hr = S_OK;
    return hr;
}

int32_t CFlvDemuxer::CreatePinOuts(IMediaType *pMT)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaFrame> spFrame = pMT->GetFrame();
    JCHK(spFrame != NULL, E_INVALIDARG);

    uint32_t count = PIN_INVALID_INDEX;
    const IMediaFrame::buf *pbuf;
    JCHK((pbuf = spFrame->GetBuf(0, &count)) != NULL, E_INVALIDARG);

    for (uint32_t i = 0; i < count; ++i) {
        uint8_t *sbuf = pbuf[i].data;
        uint32_t slen = pbuf[i].size;

        int8_t type = sbuf[0] & 0x1F;

        if (type == 0x09)
        {
            dom_ptr<IMediaType> spMT;
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMT->SetSub(MST_H264));

            bool global_header = true;

            char *value = (char*)sbuf + 11;
            int len = slen - 11 - 4;

            uint32_t w,h,codecid;
            if (SrsFlvCodec::get_h264_base_info(value, len, w, h, codecid) == S_OK)
            {
                JIF(spMT->SetVideoInfo(NULL, (int*)&w, (int*)&h, NULL, NULL, NULL));
            }

            uint8_t *buf = (uint8_t*)value + 5;
            int size = len - 5;
            JIF(spMT->SetStreamInfo(NULL,NULL,NULL, &global_header, buf, &size));

            JCHK(m_pinsOut_video.Create(CLSID_COutputPin,(IFilter*)this,false, &m_countPin),E_FAIL);
            JIF(m_pinsOut_video->SetMediaType(spMT));

            m_countPin++;
        }
        else if (type == 0x08)
        {
            int audiosamplerate,audiosamplesize,audiocodecid;
            bool stereo;

            dom_ptr<IMediaType> spMT;
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMT->SetSub(MST_AAC));
            bool global_header = true;

            char *value = (char*)sbuf + 11;
            int len = slen - 11 - 4;

            uint8_t *buf = (uint8_t*)value + 2;
            int size = len - 2;
            JIF(spMT->SetStreamInfo(NULL, NULL, NULL, &global_header, buf, &size, NULL));

            if (SrsFlvCodec::get_aac_base_info(value, len,
                  audiocodecid, audiosamplerate, audiosamplesize, stereo) == S_OK)
            {
                int channels = 1;
                if (stereo) {
                    channels = 2;
                }
                JIF(spMT->SetAudioInfo(NULL, NULL, &channels, &audiosamplerate, &audiosamplesize));
            }

            JCHK(m_pinsOut_audio.Create(CLSID_COutputPin,(IFilter*)this,false,&m_countPin),E_FAIL);
            JIF(m_pinsOut_audio->SetMediaType(spMT));

            m_countPin++;
        }
    }

    return hr;
}

int32_t CFlvDemuxer::CreatePinOuts()
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT = m_pinIn->GetMediaType(), E_FAIL);

    JIF(CreatePinOuts(spMT));

    return hr;
}

int32_t CFlvDemuxer::ProcessFlvFrame(IMediaFrame *pFrame)
{
    HRESULT hr;
    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spAllocate->Alloc(&spFrame));
    JIF(spFrame->CopyFrom(pFrame,MEDIAFRAME_COPY_INFO));

    IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(spFrame->GetBuf(0));

    uint8_t type = pBuf->data[0] & 0x1F;

    pBuf->data += 11;
    pBuf->size -= 11 + 4;

    if (type == 9) {
        bool is_264 = SrsFlvVideo::h264((char*)pBuf->data, (int)pBuf->size);
        if (is_264) {
            pBuf->data += 5;
            pBuf->size -= 5;
        } else {
            pBuf->data += 1;
            pBuf->size -= 1;
        }
        JIF(m_pinsOut_video->Write(spFrame));
    } else if (type == 8) {
        bool is_aac = SrsFlvAudio::aac((char*)pBuf->data, (int)pBuf->size);

        if (is_aac) {
            pBuf->data += 2;
            pBuf->size -= 2;
        } else {
            pBuf->data += 1;
            pBuf->size -= 1;
        }
        spFrame->info.flag |= MEDIA_FRAME_FLAG_SYNCPOINT;
        JIF(m_pinsOut_audio->Write(spFrame));
    }

    return hr;
}

int32_t CFlvDemuxer::ReadFlvHeader(IStream *pStream)
{
    HRESULT hr = S_OK;
    uint8_t buf[13];
    JIF(pStream->Read(buf,13));

    if (buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return -1;
    }
    m_type = TagHeader;

    return hr;
}

int32_t CFlvDemuxer::ReadFlvTagHeader(IStream *pStream)
{
    HRESULT hr = S_OK;

    uint8_t buf[11];
    JIF(pStream->Read(buf,11));

    int8_t type = buf[0] & 0x1F;

    // DataSize UI24
    int32_t size;
    char *p = (char*)&size;
    p[3] = 0;
    p[2] = buf[1];
    p[1] = buf[2];
    p[0] = buf[3];

    // Timestamp UI24
    int32_t timestamp;
    p = (char*)&timestamp;
    p[2] = buf[4];
    p[1] = buf[5];
    p[0] = buf[6];

    // TimestampExtended UI8
    p[3] = buf[7];

    m_header.message_type = type;
    m_header.payload_length = size;
    m_header.timestamp = timestamp;

    m_type = TagBody;

    return hr;
}

int32_t CFlvDemuxer::ReadFlvTagBody(IStream *pStream)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spAllocate->Alloc(&spFrame));
    JIF(spFrame->SetBuf(0, m_header.payload_length));

    JIF(pStream->Read(spFrame->GetBuf(0)->data, m_header.payload_length));

    CommonMessage *msg = new CommonMessage();
    unique_ptr<CommonMessage> temp(msg);

    msg->header = m_header;

    msg->setPayload(spFrame);

    JIF(process_video_audio(msg));

    m_type = PrevousTagSize;

    return hr;
}

int32_t CFlvDemuxer::ReadFlvTagPrevious(IStream *pStream)
{
    HRESULT hr = S_OK;

    uint8_t buf[4];
    JIF(pStream->Read(buf,4));

    m_type = TagHeader;

    return hr;
}

int32_t CFlvDemuxer::FlvService(IStream *pStream)
{
    HRESULT hr = S_OK;

    while (true) {
        switch (m_type) {
        case FlvHeader:
            JIF(ReadFlvHeader(pStream));
        case TagHeader:
            JIF(ReadFlvTagHeader(pStream));
        case TagBody:
            JIF(ReadFlvTagBody(pStream));
        case PrevousTagSize:
            JIF(ReadFlvTagPrevious(pStream));
        }
    }
    return hr;
}

int CFlvDemuxer::process_v_sequence(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT = m_pinIn->GetMediaType(), E_FAIL);

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spAllocate->Alloc(&spFrame));

    JIF(convert_to_flv(msg, spFrame));

    dom_ptr<IMediaFrame> pFrame;
    pFrame = spMT->GetFrame();
    if (pFrame == NULL) {
        JCHK(pFrame.Create(CLSID_CMediaFrame), E_FAIL);
        JIF(pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, flv_header_size, flv_header));
    }

    pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, spFrame->GetBuf(0)->size, spFrame->GetBuf(0)->data);

    return hr;
}

int CFlvDemuxer::process_a_sequence(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT = m_pinIn->GetMediaType(), E_FAIL);

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spAllocate->Alloc(&spFrame));

    JIF(convert_to_flv(msg, spFrame));

    dom_ptr<IMediaFrame> pFrame;
    pFrame = spMT->GetFrame();
    if (pFrame == NULL) {
        JCHK(pFrame.Create(CLSID_CMediaFrame), E_FAIL);
        JIF(pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, flv_header_size, flv_header));
    }

    pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, spFrame->GetBuf(0)->size, spFrame->GetBuf(0)->data);

    return hr;
}

int CFlvDemuxer::process_metadata(CommonMessage *msg)
{
    HRESULT hr = S_OK;

//    dom_ptr<IMediaType> spMT;
//    JCHK(spMT = m_pinIn->GetMediaType(), E_FAIL);

//    dom_ptr<IMediaFrame> spFrame;
//    JIF(convert_to_flv(msg, &spFrame));

//    dom_ptr<IMediaFrame> pFrame;
//    pFrame = spMT->GetFrame();
//    if (pFrame == NULL) {
//        JCHK(pFrame.Create(CLSID_CMediaFrame), E_FAIL);
//        JIF(pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, flv_header_size, flv_header));
//    }

//    pFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT, spFrame->GetBuf(0)->size, spFrame->GetBuf(0)->data);

    return hr;
}

int CFlvDemuxer::process_av_cache()
{
    HRESULT hr = S_OK;

    bool has_v_seq = false;
    bool has_a_seq = false;

    list<CommonMessage*>::iterator it;

    for (it = m_msgs.begin(); it != m_msgs.end(); ++it) {
        CommonMessage *msg = *it;

        if (msg->is_video()) {
            IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(msg->payload->GetBuf(0));

            has_v_seq = SrsFlvVideo::sh((char*)pBuf->data, pBuf->size);

            if (has_v_seq) {
                JIF(process_v_sequence(msg));
            }
        } else if (msg->is_audio()) {
            IMediaFrame::buf* pBuf = const_cast<IMediaFrame::buf*>(msg->payload->GetBuf(0));

            has_a_seq = SrsFlvAudio::sh((char*)pBuf->data, pBuf->size);

            if (has_a_seq) {
                JIF(process_a_sequence(msg));
            }
        } else {
            JIF(process_metadata(msg));
        }
    }

    clear();
    m_first = false;

    JIF(m_ep->Notify(ET_Filter_Build,0,(IFilter*)this));

    CreatePinOuts();

    return hr;
}

HRESULT CFlvDemuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL != pMtIn)
    {
        if(MST_FLV_TAG == pMtIn->GetSub())
            return 1;
    }
    return E_INVALIDARG;
}

int CFlvDemuxer::process_av_data(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    dom_ptr<IMediaFrame> spFrame;
    JIF(m_spAllocate->Alloc(&spFrame));

    JIF(convert_to_flv(msg, spFrame));

    JIF(ProcessFlvFrame(spFrame));

    return hr;
}

int CFlvDemuxer::process_video_audio(CommonMessage *msg)
{
    HRESULT hr = S_OK;

    if (m_first) {
        if (m_msgs.size() < 4) {
            CommonMessage *_msg = new CommonMessage(msg);
            m_msgs.push_back(_msg);
        }

        if (m_msgs.size() == 4) {
            return process_av_cache();
        }
    } else {
        JIF(process_av_data(msg));
    }

    return hr;
}

void CFlvDemuxer::clear()
{
    list<CommonMessage*>::iterator it;

    for (it = m_msgs.begin(); it != m_msgs.end(); ++it) {
        srs_freep(*it);
    }
    m_msgs.clear();
}
