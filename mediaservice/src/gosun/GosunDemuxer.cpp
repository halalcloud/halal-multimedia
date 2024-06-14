#include "GosunDemuxer.h"

CGosunDemuxer::CGosunDemuxer()
    :m_tag(NULL)
    ,m_pinsOut(NULL)
    ,m_count(0)
    ,m_status(S_Stop)
    ,m_isEOF(false)
{
    //ctor
}

bool CGosunDemuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_pinIn.Create(CLSID_CInputPin,(IFilter*)this),false);
    return true;
}

bool CGosunDemuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CGosunDemuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CGosunDemuxer::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CGosunDemuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CGosunDemuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CGosunDemuxer::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CGosunDemuxer::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CGosunDemuxer::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn : NULL;
}

STDMETHODIMP_(uint32_t) CGosunDemuxer::GetOutputPinCount()
{
    return m_count;
}

STDMETHODIMP_(IOutputPin*) CGosunDemuxer::GetOutputPin(uint32_t index)
{
    JCHK(index < m_count,NULL);
    return m_pinsOut[index];
}

STDMETHODIMP_(IInputPin*) CGosunDemuxer::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CGosunDemuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CGosunDemuxer::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(S_Stop != m_status)
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

STDMETHODIMP_(uint32_t) CGosunDemuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CGosunDemuxer::SetTag(void* pTag)
{
    m_tag = pTag;
}

STDMETHODIMP_(void*) CGosunDemuxer::GetTag()
{
    return m_tag;
}

STDMETHODIMP_(double) CGosunDemuxer::GetExpend()
{
    return 0;
}

STDMETHODIMP CGosunDemuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    JCHK(pPin == m_pinIn.p,E_FAIL);
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        if(pMT->GetSub() != MST_GOSUN_DATA)
            return E_INVALIDARG;

        dom_ptr<IMediaFrame> spFrame;
        if(NULL == (spFrame = pMT->GetFrame()))
            return E_INVALIDARG;

        dom_ptr<IStream> spStream;
        JCHK(spFrame.Query(&spStream),E_FAIL);

        JIF(Load(spStream,0));

        return S_OK;
    }
    else
    {
        if(NULL != m_pinsOut)
        {
            delete[] m_pinsOut;
            m_pinsOut = NULL;
        }
        m_count = 0;
    }
    return hr;
}

STDMETHODIMP CGosunDemuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return NULL == pMT || COMPARE_SAME_VALUE == pPin->GetMediaType()->Compare(pMT) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CGosunDemuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    JCHK(m_pinIn == pPin,E_INVALIDARG);
    JCHK(NULL != pFrame,E_INVALIDARG);

    HRESULT hr = S_OK;
    if(NULL != pFrame->GetBuf())
    {
//        if(MEDIA_FRAME_FLAG_SYNCPOINT < pFrame->info.flag)
//        {
//            //printf("CGosunDemuxer::OnWriteFrame dts:%ld flag:%d\n",pFrame->info.dts,pFrame->info.flag);
//            for(uint32_t i=0 ; i<m_count ; ++i)
//            {
//                JIF(m_pinsOut[i]->SetFlag(pFrame->info.flag));
//            }
//        }
        dom_ptr<IStream> spStream;
        JCHK(spStream.QueryFrom(pFrame),E_FAIL);

        int64_t pos = spStream->GetPos();
        while(true)
        {
            uint8_t id;
            JIF(spStream->Read(&id,sizeof(id)));
            if(MAX_PACKET_ID == id)
                break;
            JCHK(id < m_count,E_FAIL);

            dom_ptr<IMediaFrame> spFrame;
            dom_ptr<ISerialize> spSerialize;
            JIF(m_pinsOut[id]->AllocFrame(&spFrame));
            JCHK(spFrame.Query(&spSerialize),E_FAIL);
            JIF(spSerialize->Load(spStream,0,&pFrame->info.dts));
            JIF(m_pinsOut[id]->Write(spFrame));
        }
        JIF(spStream->Seek(pos));
    }
    if(0 != (pFrame->info.flag&MEDIA_FRAME_FLAG_EOF))
    {
        pFrame->Clear();
        for(uint32_t i=0 ; i<m_count ; ++i)
        {
            JIF(m_pinsOut[i]->Write(pFrame));
        }
    }
    return hr;
}

HRESULT CGosunDemuxer::Load(IStream* pStream,uint8_t flag)
{
    HRESULT hr;
    if(NULL != m_pinsOut)
    {
        delete[] m_pinsOut;
        m_pinsOut = NULL;
    }
    m_count = 0;
    JIF(pStream->Read(&m_count,sizeof(m_count),flag));
    JCHK(m_pinsOut = new dom_ptr<IOutputPin>[m_count],E_OUTOFMEMORY);
    for(uint32_t i=0 ; i<m_count ; ++i)
    {
        dom_ptr<IMediaType> spMT;
        dom_ptr<ISerialize> spSerialize;
        JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
        JCHK(spMT.Query(&spSerialize),E_FAIL);
        JIF(spSerialize->Load(pStream,flag));
        JCHK(m_pinsOut[i].Create(CLSID_COutputPin,this,false,&i),E_FAIL);
        JIF(m_pinsOut[i]->SetMediaType(spMT));
        m_pinsOut[i]->SetID(i);
    }
    return hr;
}

HRESULT CGosunDemuxer::Open()
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT CGosunDemuxer::Close()
{
    HRESULT hr = S_OK;
    return hr;
}
HRESULT CGosunDemuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL != pMtIn)
    {
        if(MST_GOSUN_DATA == pMtIn->GetSub())
            return 1;
    }
    return E_INVALIDARG;
}
