#include "IntelVideoDecoder.h"

CIntelVideoDecoder::CIntelVideoDecoder()
:m_pFG(NULL)
,m_isOpen(false)
,m_pTag(NULL)
,m_pMtIn(NULL)
,m_pMtOut(NULL)
,m_isFirst(false)
,m_pix_fmt(VMT_NONE)
,m_width(0)
,m_height(0)
,m_ratioX(0)
,m_ratioY(0)
,m_align(0)
,m_duration(0)
{
    //ctor
}

bool CIntelVideoDecoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
    return true;
}

bool CIntelVideoDecoder::FinalDestructor(bool finally)
{
    if(true == finally)
        Close();
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CIntelVideoDecoder)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CIntelVideoDecoder::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CIntelVideoDecoder::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CIntelVideoDecoder::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CIntelVideoDecoder::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CIntelVideoDecoder::GetInputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinIn;
}

STDMETHODIMP_(uint32_t) CIntelVideoDecoder::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CIntelVideoDecoder::GetOutputPin(uint32_t index)
{
    JCHK(0 == index,NULL);
    return m_spPinOut;
}

STDMETHODIMP CIntelVideoDecoder::Open()
{
    HRESULT hr = S_OK;
    JCHK(NULL != m_pMtIn,E_FAIL);
    JCHK(NULL != m_pMtOut,E_FAIL);
    if(false == m_isOpen)
    {
        //add initlize code


        JCHK(m_spFrame.Create(CLSID_CMediaFrame),E_FAIL);
        m_isFirst = true;
        m_isOpen = true;
    }
	return hr;
}

STDMETHODIMP CIntelVideoDecoder::Close()
{
    if(true == m_isOpen)
    {
        //add uninitlize code



        m_isOpen = false;
    }
    return S_OK;
}

STDMETHODIMP CIntelVideoDecoder::SetTag(void* pTag)
{
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CIntelVideoDecoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CIntelVideoDecoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CIntelVideoDecoder::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CIntelVideoDecoder::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    if(NULL == m_pMtIn)
        return E_FAIL;
    else
    {
        HRESULT hr;
        JIF(pMT->Clear());
        JIF(pMT->SetSub(MST_RAWVIDEO));
        JIF(pMT->SetVideoInfo(&m_pix_fmt,
            &m_width,
            &m_height,
            &m_ratioX,
            &m_ratioY,
            &m_duration));
        return hr;
    }
}


STDMETHODIMP CIntelVideoDecoder::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT) //connect
    {
		JIF(CheckMediaType(pMT,m_pMtOut));
        JIF(pMT->GetVideoInfo(&m_pix_fmt,
            &m_width,
            &m_height,
            &m_ratioX,
            &m_ratioY,
            &m_duration));
    }
    else //break connect
    {

    }
    m_pMtIn = pMT;
    return hr;
}

STDMETHODIMP CIntelVideoDecoder::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        JIF(CheckMediaType(m_pMtIn,pMT));
        pMT->GetVideoInfo(&m_pix_fmt,
            &m_width,
            &m_height,
            &m_ratioX,
            &m_ratioY,
            &m_duration);
        if(VMT_NV12 != m_pix_fmt)
        {
            JIF(pMT->SetVideoInfo(&m_pix_fmt));
        }
    }
    m_pMtOut = pMT;
    return hr;
}

STDMETHODIMP CIntelVideoDecoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    HRESULT hr;
    if(false == m_isOpen)
    {
        if(NULL != pFrame)
        {
            JIF(Open());
        }
    }
    if(true == m_isOpen)
    {
        hr = Write(pFrame);
        if(S_STREAM_EOF == hr)
            Close();
    }
    if(false == m_isOpen)
        hr = m_spPinOut->Write(pFrame);
    return hr;
}

STDMETHODIMP CIntelVideoDecoder::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

HRESULT CIntelVideoDecoder::Write(IMediaFrame* pFrame)
{
	HRESULT hr = S_OK;
	if(pFrame == NULL)//flush
	{
	}
	else
	{
        //printf("\nvideo decoder frame[DTS:%dms,PTS:%dms]\n",pFrame->info.dts,pFrame->info.pts);
        if(true == m_isFirst)
        {
            if(0 == (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
            {
                LOG(4,"video decoder first input frame[DTS:%dms] is not key frame and will be dropped",pFrame->info.dts);
                return S_OK;
            }
            m_isFirst = false;
        }
	}
	//add decode code

    return hr;
}

HRESULT CIntelVideoDecoder::CheckMediaType(IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr;
	if(NULL == pMtIn)
		return E_INVALIDARG;

	if(MST_H264 != pMtIn->GetSub())
		return E_INVALIDARG;

    int width_in = 0,height_in = 0;
    int64_t duration_in = 0;

    JIF(pMtIn->GetVideoInfo(NULL,&width_in,&height_in,NULL,NULL,&duration_in));
	if(NULL != pMtOut)
	{
        if(MST_RAWVIDEO != pMtOut->GetSub())
            return E_INVALIDARG;

        VideoMediaType vmt_out = VMT_NONE;
        int width_out = 0,height_out = 0;
        int64_t duration_out = 0;

        JIF(pMtOut->GetVideoInfo(&vmt_out,&width_out,&height_out,NULL,NULL,&duration_out));
        if(VMT_NV12 != vmt_out)
            return E_INVALIDARG;
        if(width_in != width_out || height_in != height_out || duration_in != duration_out)
            return E_INVALIDARG;
	}
	return 1;
}

