#include "IntelVideoDecoder.h"

CIntelVideoDecoder::CIntelVideoDecoder()
:m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
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
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spPinIn.Create(CLSID_CInputPin,(IFilter*)this),false);
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
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CIntelVideoDecoder::GetType()
{
    return FT_Transform;
}

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
    return m_spPinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CIntelVideoDecoder::GetInputPin(uint32_t index)
{
    return 0 == index ? m_spPinIn : NULL;
}

STDMETHODIMP_(uint32_t) CIntelVideoDecoder::GetOutputPinCount()
{
    return m_spPinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CIntelVideoDecoder::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_spPinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CIntelVideoDecoder::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CIntelVideoDecoder::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CIntelVideoDecoder::Notify(uint32_t cmd)
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
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    else if(cmd < C_NB)
    {
        if(C_Flush == cmd)
        {
            do
            {
                hr = Write(NULL);
            }while(hr >= S_OK);
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CIntelVideoDecoder::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CIntelVideoDecoder::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CIntelVideoDecoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CIntelVideoDecoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CIntelVideoDecoder::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT) //connect
    {
        JIF(FilterQuery(NULL,NULL,pMT,m_spPinOut == NULL ? NULL : m_spPinOut->GetMediaType()));
        JIF(pMT->GetVideoInfo(&m_pix_fmt,
            &m_width,
            &m_height,
            &m_ratioX,
            &m_ratioY,
            &m_duration));

        if(m_spPinOut == NULL)
        {
            dom_ptr<IMediaType> spMT;
            JCHK(m_spPinOut.Create(CLSID_COutputPin,this),false);
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMT->SetSub(MST_RAWVIDEO));
            JIF(spMT->SetVideoInfo(&m_pix_fmt,
                &m_width,
                &m_height,
                &m_ratioX,
                &m_ratioY,
                &m_duration));
            JIF(m_spPinOut->SetMediaType(spMT));
        }
    }
    return hr;
}

STDMETHODIMP CIntelVideoDecoder::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        JIF(FilterQuery(NULL,NULL,m_spPinIn->GetMediaType(),pMT));
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
    return hr;
}

STDMETHODIMP CIntelVideoDecoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Write(pFrame);
}

HRESULT CIntelVideoDecoder::Open()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
    {
        //add initlize code


        JCHK(m_spFrame.Create(CLSID_CMediaFrame),E_FAIL);
        m_isFirst = true;
        m_isOpen = true;
    }
	return hr;
}

HRESULT CIntelVideoDecoder::Close()
{
    if(true == m_isOpen)
    {
        //add uninitlize code



        m_isOpen = false;
    }
    return S_OK;
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

HRESULT CIntelVideoDecoder::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
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

