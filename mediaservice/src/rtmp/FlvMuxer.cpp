#include "FlvMuxer.h"
#include "SrsFlvCodec.hpp"
#include "bitstreamfilter.h"
#include "SrsFlvCodec.hpp"
#include "RtmpGlobal.hpp"

CFlvMuxer::CFlvMuxer()
:m_status(S_Stop)
,m_isOpen(NULL)
,m_isFirst(false)
,m_pTag(NULL)
,m_index(0)
,m_master(0)
,m_v_sequence(false)
,m_a_sequence(false)
,m_v_convert(true)
,m_a_convert(true)
,m_base(0)
{
    //ctor
}


bool CFlvMuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    dom_ptr<IMediaType> spMT;
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this),false);
    JCHK(spMT.Create(CLSID_CMediaType),false);
    JCHK(S_OK == spMT->SetSub(MST_FLV_TAG),false);
    JCHK(m_pinOut.Create(CLSID_COutputPin,(IFilter*)this),false);

    JCHK(S_OK == m_pinOut->SetMediaType(spMT),false);

    JCHK(m_spAllocate.Create(CLSID_CMediaFrameAllocate),false);
    return true;
}

bool CFlvMuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFlvMuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CFlvMuxer::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CFlvMuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CFlvMuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CFlvMuxer::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CFlvMuxer::GetInputPinCount()
{
    return m_inputs.size();
}

STDMETHODIMP_(IInputPin*) CFlvMuxer::GetInputPin(uint32_t index)
{
    return index < m_inputs.size() ? m_inputs.at(index) : NULL;
}

STDMETHODIMP_(uint32_t) CFlvMuxer::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CFlvMuxer::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CFlvMuxer::CreateInputPin(IMediaType* pMT)
{
    JCHK(false == m_isOpen,NULL);
    dom_ptr<IInputPin> spPin;
    size_t index = m_inputs.size();
    JCHK(spPin.Create(CLSID_CInputPin,this,false,&index),NULL);
    if(IS_FAIL(OnConnect(spPin,NULL,pMT)))
        return NULL;
    if(IS_FAIL(spPin->SetMediaType(pMT)))
        return NULL;
    m_inputs.push_back(spPin);
    return spPin;
}

STDMETHODIMP_(IOutputPin*) CFlvMuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CFlvMuxer::Notify(uint32_t cmd)
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
                for(uint32_t i=0 ; i<m_inputs.size() ; ++i)
                {
                    dom_ptr<IInputPin>& pin = m_inputs.at(i);
                    pin->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                }
                m_index = 0;
                m_isFirst = true;
            }
            m_status = (Status)cmd;
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CFlvMuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CFlvMuxer::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CFlvMuxer::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CFlvMuxer::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CFlvMuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    if(NULL != pMT)
    {
        JCHK(MMT_VIDEO == pMT->GetMajor() || MMT_AUDIO == pMT->GetMajor(),E_INVALIDARG);
        JCHK(MST_NONE != pMT->GetSub(),E_INVALIDARG);
    }
    return S_OK;
}

STDMETHODIMP CFlvMuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    if(NULL != pMT)
        return MST_FLV_TAG == pMT->GetSub() ? S_OK : E_INVALIDARG;
    else
        return S_OK;
}

STDMETHODIMP CFlvMuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    HRESULT hr;
    if(NULL != pPin)
    {
        dom_ptr<IMediaType> spMT;
        JCHK(spMT = pPin->GetMediaType(),E_INVALIDARG);
        MediaMajorType mmt = spMT->GetMajor();

        dom_ptr<IMediaFrame> spFrame;
        JIF(m_spAllocate->Alloc(&spFrame));

        dom_ptr<IMediaFrame> spHeader;
        JCHK(spHeader = m_pinOut->GetMediaType()->GetFrame(),E_FAIL);
        if(MEDIA_FRAME_NONE_TIMESTAMP == spHeader->info.dts)
        {
            spHeader->info.dts = 0;
            spHeader->info.pts = 0;
            m_base = pFrame->info.dts;
        }


        spFrame->info = pFrame->info;
        spFrame->info.dts -= m_base;
        spFrame->info.pts -= m_base;

        if(MMT_VIDEO == mmt)
        {
            IMediaFrame::buf* pBuf;
            JCHK(pBuf = const_cast<IMediaFrame::buf*>(pFrame->GetBuf(0)),E_INVALIDARG);

            bool is_264 = MST_H264 == spMT->GetSub();
            bool is_Key = 0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT);

            if (is_264 && m_v_convert) {
                uint8_t *buf = NULL;
                uint32_t size = 0;
                h264_annexbtomp4 h264_convert;
                h264_convert.process(pBuf->data, pBuf->size, buf, size);
                unique_ptr<uint8_t,h264_annexbtomp4::buf_deleter> temp(buf,h264_annexbtomp4::buf_deleter(h264_convert));

                if (!m_v_sequence) {

                    uint8_t *c_buf = NULL;
                    uint32_t c_size = 0;
                    h264_convert.getAvcDecodeConfigRecord(c_buf, c_size);

                    JIF(spHeader->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT,c_size + 11 + 5 + 4));
                    generate_flv_sm_tag(spHeader->GetBuf((uint32_t)hr), c_buf, c_size, 0x09);
                    m_v_sequence = true;
                }

                JIF(generate_flv_tag(spFrame, true, is_264, false, buf, size, spFrame->info.dts, spFrame->info.pts, is_Key));
            } else {
                JIF(generate_flv_tag(spFrame, true, is_264, false, pBuf->data, pBuf->size, spFrame->info.dts, spFrame->info.pts, is_Key));
            }

            JIF(m_pinOut->Write(spFrame));
        }
        else if(MMT_AUDIO == mmt)
        {
            IMediaFrame::buf* pBuf;
            JCHK(pBuf = const_cast<IMediaFrame::buf*>(pFrame->GetBuf(0)),E_INVALIDARG);

            bool is_aac = MST_AAC == spMT->GetSub();

            if (is_aac && m_a_convert) {
                uint32_t offset = 0;
                aac_adts2asc aac_convert;
                JIF(aac_convert.process(pBuf->data, pBuf->size, offset));

                if (!m_a_sequence) {

                    uint8_t s_buf[2];
                    uint32_t s_len = 2;
                    aac_convert.getDsi(s_buf, s_len);

                    JIF(spHeader->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT,s_len + 11 + 2 + 4));
                    generate_flv_sm_tag(spHeader->GetBuf((uint32_t)hr), s_buf, s_len, 0x08);
                    m_a_sequence = true;
                }

                JIF(generate_flv_tag(spFrame, false, false, is_aac, pBuf->data + offset, pBuf->size - offset, spFrame->info.dts, spFrame->info.pts, false));
            } else {
                JIF(generate_flv_tag(spFrame, false, false, is_aac, pBuf->data, pBuf->size, spFrame->info.dts, spFrame->info.pts, false));
            }
            spFrame->info.flag &= ~MEDIA_FRAME_FLAG_SYNCPOINT;
            JIF(m_pinOut->Write(spFrame));
        }
    }

    return hr;
}

HRESULT CFlvMuxer::Open()
{
    HRESULT hr = S_OK;

    if(true == m_isOpen)
        return hr;

    dom_ptr<IMediaType> spMtOut;
    JCHK(spMtOut = m_pinOut->GetMediaType(),E_FAIL);

    dom_ptr<IMediaFrame> spFrame;
    JCHK(spFrame.Create(CLSID_CMediaFrame),E_FAIL);
    spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT;
    JIF(spFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT,flv_header_size,flv_header,IStream::WRITE_FLAG_REFFER));

    uint32_t count = GetInputPinCount();
    for(uint32_t i=0 ; i<count ; ++i)
    {
        IInputPin* pPin;
        IOutputPin* pPinOut;
        JCHK(pPin = GetInputPin(i),E_FAIL);
        if(NULL != (pPinOut = pPin->GetConnection()))
        {
            dom_ptr<IMediaType> spMt;
            JCHK(spMt = pPin->GetMediaType(),E_FAIL);
            bool global_header = false;
            uint8_t* extra_data = NULL;
            int extra_size = 0;
            spMt->GetStreamInfo(NULL,NULL,NULL,&global_header,&extra_data,&extra_size);
            if(true == global_header)
            {
                JCHK(NULL != extra_data && 0 < extra_size,E_FAIL);
                if(MMT_VIDEO == spMt->GetMajor())
                {
                    JIF(spFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT,extra_size + 11 + 5 + 4));
                    generate_flv_sm_tag(spFrame->GetBuf((uint32_t)hr), extra_data, extra_size, 0x09);
                    m_v_sequence = true;
                    m_v_convert = false;
                }
                else if(MMT_AUDIO == spMt->GetMajor())
                {
                    JIF(spFrame->SetBuf(MAX_MEDIA_FRAME_BUF_COUNT,extra_size + 2 + 11 + 4));
                    generate_flv_sm_tag(spFrame->GetBuf((uint32_t)hr), extra_data, extra_size, 0x08);
                    m_a_sequence = true;
                    m_a_convert = false;
                }
            }
        }
    }
    JIF(spMtOut->SetFrame(spFrame));
    //JIF(m_pinOut->SetThrottle(true));
    m_base = 0;
    m_isOpen = true;
    return hr;
}

HRESULT CFlvMuxer::Close()
{
    HRESULT hr = S_OK;

    if(false == m_isOpen)
        return hr;

    //JIF(m_pinOut->SetThrottle(false));
    m_indexs.clear();
    m_isOpen = false;
    return hr;
}

HRESULT CFlvMuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL != pMtOut)
    {
        if(MST_FLV_TAG == pMtOut->GetSub())
            return 1;
    }
    return E_INVALIDARG;
}

void CFlvMuxer::generate_flv_sm_tag(const IMediaFrame::buf *pBuf, uint8_t *extra_data, int extra_size, char type)
{
    int len = extra_size;
    if (type == 0x08) {
        len += 2;
    } else if (type == 0x09) {
        len += 5;
    }

    char tag_header[11];
    char *p = tag_header;
    *p++ = type;

    char *pp = (char*)&len;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];

    // timestamp
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    memcpy(pBuf->data, tag_header, 11);

    char pre_tag[4];
    p = pre_tag;
    int pre_len = 11 + len;
    pp = (char*)&pre_len;
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];

    if (type == 0x08) {
        char header[2] = {0xaf, 0x00};
        memcpy(pBuf->data + 11, header, 2);
        memcpy(pBuf->data + 11 + 2, extra_data, extra_size);
        memcpy(pBuf->data + 11 + len, pre_tag, 4);
    } else if (type == 0x09) {
        char header[5] = {0x17, 0x00, 0x00, 0x00, 0x00};
        memcpy(pBuf->data + 11, header, 5);
        memcpy(pBuf->data + 11 + 5, extra_data, extra_size);
        memcpy(pBuf->data + 11 + len, pre_tag, 4);
    } else if (type == 0x12) {
        memcpy(pBuf->data + 11, extra_data, extra_size);
        memcpy(pBuf->data + 11 + len, pre_tag, 4);
    }
}

int CFlvMuxer::generate_flv_tag(IMediaFrame *pFrame, bool is_video, bool is_264, bool is_aac,
                                uint8_t *buf, int size, int64_t dts, int64_t pts, bool is_keyframe)
{
    HRESULT hr = S_OK;

    dts/=10000;
    pts/=10000;

    int len;
    char tag_header[11];
    char *p = tag_header;

    if (is_video) {
        *p++ = 0x09;

        if (is_264) {
            len = size + 5;
        } else {
            len = size + 1;
        }
    } else {
        *p++ = 0x08;

        if (is_aac) {
            len = size + 2;
        } else {
            len = size + 1;
        }
    }

    char *pp = (char*)&len;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];

    int timestamp = (int)dts;
    pp = (char*)&timestamp;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
    *p++ = pp[3];

    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    int tag_size = len + 11 + 4;
    const IMediaFrame::buf* pBuf;
    JIF(pFrame->SetBuf(0,(uint32_t)tag_size));
    JCHK(pBuf = pFrame->GetBuf(),E_FAIL);
    uint8_t* buf_tag = (uint8_t*)pBuf->data;

    memcpy(buf_tag,tag_header,11);
    buf_tag += 11;
    tag_size -= 11;
    JCHK(0 <= tag_size,E_FAIL);

    if (is_video) {
        if (is_264)
        {
            //add global header video sequence
            char body_header[2];
            if (true == is_keyframe)
            {
                body_header[0] = 0x17;
            } else {
                body_header[0] = 0x27;
            }
            body_header[1]  = 0x01;
            memcpy(buf_tag,body_header,2);
            buf_tag +=2;
            tag_size -= 2;
            JCHK(0 <= tag_size,E_FAIL);

            int composition_time = pts - dts;
            char *p = (char*)&composition_time;
            char c_buf[3];
            c_buf[0] = p[2];
            c_buf[1] = p[1];
            c_buf[2] = p[0];

            memcpy(buf_tag,c_buf,3);
            buf_tag +=3;
            tag_size -= 3;
            JCHK(0 <= tag_size,E_FAIL);
        } else {

        }
    } else {
        if (is_aac)
        {
            //add global header audio sequence
            char body_header[2] = { 0xaf, 0x01 };

            memcpy(buf_tag,body_header,2);
            buf_tag +=2;
            tag_size -= 2;
            JCHK(0 <= tag_size,E_FAIL);
        } else {

        }
    }

    memcpy(buf_tag,buf,size);
    buf_tag +=size;
    tag_size -= size;
    JCHK(0 <= tag_size,E_FAIL);

    char pre_tag[4];
    p = pre_tag;
    int pre_len = 11 + len;
    pp = (char*)&pre_len;
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];

    memcpy(buf_tag,pre_tag,4);
    buf_tag +=4;
    tag_size -= 4;
    JCHK(0 == tag_size,E_FAIL);

    return hr;
}

