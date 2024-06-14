#include "GosunMuxer.h"

CGosunMuxer::CGosunMuxer()
    :m_status(S_Stop)
    ,m_isOpen(NULL)
    ,m_pTag(NULL)
    ,m_count(0)
    ,m_start_time(MEDIA_FRAME_NONE_TIMESTAMP)
    ,m_delta_time(MEDIA_FRAME_NONE_TIMESTAMP)
{
    //ctor
}


bool CGosunMuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    dom_ptr<IMediaType> spMT;
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this),false);
    JCHK(spMT.Create(CLSID_CMediaType),false);
    JCHK(S_OK == spMT->SetSub(MST_GOSUN_DATA),false);
    JCHK(m_pinOut.Create(CLSID_COutputPin,(IFilter*)this),false);
    JCHK(S_OK == m_pinOut->SetMediaType(spMT),false);
    return true;
}

bool CGosunMuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CGosunMuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CGosunMuxer::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CGosunMuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CGosunMuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CGosunMuxer::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CGosunMuxer::GetInputPinCount()
{
    return m_inputs.size();
}

STDMETHODIMP_(IInputPin*) CGosunMuxer::GetInputPin(uint32_t index)
{
    return index < m_inputs.size() ? m_inputs.at(index) : NULL;
}

STDMETHODIMP_(uint32_t) CGosunMuxer::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CGosunMuxer::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CGosunMuxer::CreateInputPin(IMediaType* pMT)
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

STDMETHODIMP_(IOutputPin*) CGosunMuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CGosunMuxer::Notify(uint32_t cmd)
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
                m_pinOut->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
            }
            m_status = (Status)cmd;
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CGosunMuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CGosunMuxer::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CGosunMuxer::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CGosunMuxer::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CGosunMuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);

    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        JCHK(MMT_VIDEO == pMT->GetMajor() || MMT_AUDIO == pMT->GetMajor(),E_INVALIDARG);
        JCHK(MST_NONE != pMT->GetSub(),E_INVALIDARG);
        if(MMT_VIDEO == pMT->GetMajor())
        {
    //        int width,height;
    //        int64_t duration;
    //        JIF(pMT->GetVideoInfo(NULL,&width,&height,NULL,NULL,&duration));
        }
        else if(MMT_AUDIO == pMT->GetMajor())
        {
    //        int channels,sample_rate,frame_size;
    //        JIF(pMT->GetAudioInfo(NULL,NULL,&channels,&sample_rate,&frame_size));
        }
    }
    return hr;
}

STDMETHODIMP CGosunMuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    if(NULL != pMT)
    {
        if(MST_NONE == pMT->GetSub())
            return pMT->SetSub(MST_GOSUN_DATA);
        else
            return MST_GOSUN_DATA == pMT->GetSub() ? S_OK : E_INVALIDARG;
    }
    else
        return S_OK;
}

STDMETHODIMP CGosunMuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Deliver(pPin->GetID(),pFrame);
}

HRESULT CGosunMuxer::Deliver(uint32_t id,IMediaFrame* pFrame)
{
    HRESULT hr;
    JCHK(id < MAX_PACKET_ID,E_INVALIDARG);

    CLocker locker(m_locker);
    if(NULL != pFrame)
    {
        if(0 == id)
        {
            if(m_frame == NULL)
            {
                m_pinOut->GetMediaType()->GetStreamInfo(&m_start_time);
                if(MEDIA_FRAME_NONE_TIMESTAMP != m_start_time)
                    m_delta_time = pFrame->info.dts -m_start_time;
                else
                    m_delta_time = 0;
            }
            else
            {
                dom_ptr<IStream> spStream;
                JCHK(m_frame.Query(&spStream),E_FAIL);
                JIF(spStream->Write(&MAX_PACKET_ID,sizeof(MAX_PACKET_ID)));
                m_frame->info.pts = m_frame->info.dts;
                JIF(Output(m_frame));
                m_frame = NULL;
            }
            JIF(m_pinOut->AllocFrame(&m_frame));
            m_frame->info = pFrame->info;
            m_frame->info.pts = m_frame->info.dts;
            m_frame->info.dts -= m_delta_time;
        }
        else
        {
            if(m_frame == NULL)
                return S_OK;
        }

        dom_ptr<IStream> spStream;
        JCHK(m_frame.Query(&spStream),E_FAIL);
        uint8_t id_uint8_t = (uint8_t)id;
        JIF(spStream->Write(&id_uint8_t,sizeof(id_uint8_t)));

        dom_ptr<ISerialize> spSerialize;
        JCHK(spSerialize.QueryFrom(pFrame),E_FAIL);
        JIF(spSerialize->Save(spStream,IStream::WRITE_FLAG_REFFER,&m_frame->info.pts));
    }
    else
    {
        if(m_frame != NULL)
        {
            dom_ptr<IStream> spStream;
            JCHK(m_frame.Query(&spStream),E_FAIL);
            JIF(spStream->Write(&MAX_PACKET_ID,sizeof(MAX_PACKET_ID)));
            JIF(Output(m_frame));
            m_frame = NULL;
        }
        hr = E_EOF;
    }
    return hr;
}

HRESULT CGosunMuxer::Open()
{
    HRESULT hr = S_OK;

    if(true == m_isOpen)
        return hr;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT = m_pinOut->GetMediaType(),E_FAIL);

    dom_ptr<IProfile> spProfile;
    JCHK(spMT.Query(&spProfile),false);


    dom_ptr<IMediaFrame> spFrame;
    dom_ptr<IStream> spStream;
    JIF(m_pinOut->AllocFrame(&spFrame));
    JCHK(spFrame.Query(&spStream),E_FAIL);

    m_count = 0;
    JIF(spStream->Write(&m_count,sizeof(m_count),IStream::WRITE_FLAG_REFFER));
    for(uint32_t i=0 ; i<m_inputs.size() ; ++i)
    {
        dom_ptr<IInputPin>& pin = m_inputs.at(i);
        if(NULL != pin->GetConnection())
        {
            dom_ptr<ISerialize> spSerialize;
            JCHK(spSerialize.QueryFrom(pin->GetMediaType()),E_FAIL);
            JIF(spSerialize->Save(spStream,IStream::WRITE_FLAG_REFFER));
            pin->SetID(m_count++);
        }
    }
    JIF(spMT->SetFrame(spFrame));
    m_start_time = MEDIA_FRAME_NONE_TIMESTAMP;
    m_delta_time = MEDIA_FRAME_NONE_TIMESTAMP;
    m_isOpen = true;
    return hr;
}

HRESULT CGosunMuxer::Close()
{
    HRESULT hr = S_OK;

    if(false == m_isOpen)
        return hr;
    m_isOpen = false;
    return hr;
}

HRESULT CGosunMuxer::Output(IMediaFrame* pFrame)
{
    return m_pinOut->Write(pFrame);
}

HRESULT CGosunMuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL != pMtOut)
    {
        if(MST_GOSUN_DATA == pMtOut->GetSub())
            return 1;
    }
    return E_INVALIDARG;
}

