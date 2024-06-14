#include "IntelVideoEncoder.h"

// Extensions for internal use, normally these macros are blank
#ifdef MOD_ENC
#include "extension_macros.h"
#else
#define MOD_ENC_CREATE_PIPELINE
#define MOD_ENC_PRINT_HELP
#define MOD_ENC_PARSE_INPUT
#endif
void CIntelVideoEncoder::GetOption(IProfile* pProfile)
{
    IProfile::val* pVal;
    IProfile::Name name = NULL;
    while(NULL != (name = pProfile->Next(name)))
    {
        if(NULL != (pVal = pProfile->Read(name)) && NULL != (pVal->value))
        {
            if(true == STR_CMP(name,"preset"))
            {
                if(true == STR_CMP(pVal->type,typeid(int).name()) ||
                   true == STR_CMP(pVal->type,typeid(int64_t).name()))
                {
                    Params.nTargetUsage = *(mfxU16*)pVal->value;
                    if(MFX_TARGETUSAGE_1 > Params.nTargetUsage)
                        Params.nTargetUsage = MFX_TARGETUSAGE_1;
                    else if(MFX_TARGETUSAGE_7 < Params.nTargetUsage)
                        Params.nTargetUsage = MFX_TARGETUSAGE_7;
                }
                else if(true == STR_CMP(pVal->type,typeid(char*).name())||
                    true == STR_CMP(pVal->type,typeid(const char*).name()))
                {

                }
            }
            else if(true == STR_CMP(name,"gop_size"))
            {
                if(true == STR_CMP(pVal->type,typeid(int).name()) ||
                   true == STR_CMP(pVal->type,typeid(int64_t).name()))
                {
                    Params.nGopPicSize = *(mfxU16*)pVal->value;
                    if(10 > Params.nGopPicSize)
                        Params.nGopPicSize = 10;
                }
            }
            else
            {
                LOG(0,"option:%s is not support",name);
            }
        }
    }
}
static CEncodingPipeline* CreatePipeline(const sInputParams& params)
{
    MOD_ENC_CREATE_PIPELINE;

    if(params.UseRegionEncode)
    {
        //return new CRegionEncodingPipeline;
    }
    else if(params.nRotationAngle)
    {
        //return new CUserPipeline;
    }
    else
    {
        return new CEncodingPipeline;
    }
}
CIntelVideoEncoder::CIntelVideoEncoder()
:m_status(S_Stop)
,m_isOpen(false)
,m_pTag(NULL)
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

bool CIntelVideoEncoder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    JCHK(m_spPinOut.Create(CLSID_COutputPin,(IFilter*)this),false);
    return true;
}

bool CIntelVideoEncoder::FinalDestructor(bool finally)
{
    if(true == finally)
        Close();
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CIntelVideoEncoder)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CIntelVideoEncoder::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CIntelVideoEncoder::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CIntelVideoEncoder::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CIntelVideoEncoder::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CIntelVideoEncoder::GetInputPinCount()
{
    return m_spPinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CIntelVideoEncoder::GetInputPin(uint32_t index)
{
    return 0 == index ? m_spPinIn : NULL;
}

STDMETHODIMP_(uint32_t) CIntelVideoEncoder::GetOutputPinCount()
{
    return m_spPinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CIntelVideoEncoder::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_spPinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CIntelVideoEncoder::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CIntelVideoEncoder::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CIntelVideoEncoder::Notify(uint32_t cmd)
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
                m_spPinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                m_spPinOut->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
            }
            m_status = (IFilter::Status)cmd;
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

STDMETHODIMP_(uint32_t) CIntelVideoEncoder::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CIntelVideoEncoder::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CIntelVideoEncoder::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CIntelVideoEncoder::GetExpend()
{
    return 0;
}

STDMETHODIMP CIntelVideoEncoder::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT) //connect
    {
        JIF(FilterQuery(NULL,NULL,pMT,m_spPinOut->GetMediaType()));
        JIF(pMT->GetVideoInfo(&m_pix_fmt,
            &m_width,
            &m_height,
            &m_ratioX,
            &m_ratioY,
            &m_duration));
    }
    return hr;
}

static char* DST = "enc.xxx";
STDMETHODIMP CIntelVideoEncoder::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT) //connect
    {
        JIF(FilterQuery(NULL,NULL,m_spPinIn == NULL ? NULL : m_spPinIn->GetMediaType(),pMT));
        JIF(pMT->GetVideoInfo(&m_pix_fmt,
            &m_width,
            &m_height,
            &m_ratioX,
            &m_ratioY,
            &m_duration));

        if(VMT_NV12 != m_pix_fmt)
        {
            m_pix_fmt = VMT_NV12;
            pMT->SetVideoInfo(&m_pix_fmt);

        }
        int64_t bit_rate = 20000;
        bool isGlobalHeader = false;
        pMT->GetStreamInfo(NULL,NULL,&bit_rate);
        JIF(pMT->SetStreamInfo(NULL,NULL,NULL,&isGlobalHeader,NULL,NULL));

        memset(&Params, 0, sizeof(Params));
        Params.ColorFormat = MFX_FOURCC_YV12;
        Params.nWidth = Params.nDstWidth = m_width;
        Params.nHeight = Params.nDstHeight = m_height;
        Params.dFrameRate = 1000.0/(m_duration/10000);
        Params.bUseHWLib = true;
        Params.memType = SYSTEM_MEMORY;
        Params.CodecId = MFX_CODEC_AVC;
        Params.nTargetUsage = 4;
        Params.nBitRate = bit_rate/1000;
        Params.numViews = 1;
        Params.nAsyncDepth = 4;
        Params.nRateControlMethod = 1;
        Params.nPicStruct = 1;
        Params.nGopPicSize = 250;
        Params.dstFileBuff.push_back(DST);
        char SO[] = "";
        //memcpy(Params.pluginParams.strPluginPath, SO, sizeof(SO);
        memset(Params.strSrcFile, 0, MSDK_MAX_FILENAME_LEN);
        memset(Params.strPluginDLLPath, 0, MSDK_MAX_FILENAME_LEN);
        MediaSubType sub = pMT->GetSub();
        if(MST_H264 == sub)
        {
            Params.CodecId = MFX_CODEC_AVC;
        }
        else if (MST_H265 == sub)
        {
            Params.CodecId = MFX_CODEC_HEVC;
        }
        if(m_spPinIn == NULL)
        {
            dom_ptr<IMediaType> spMT;
            JCHK(m_spPinIn.Create(CLSID_CInputPin,this),false);
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMT->SetSub(MST_RAWVIDEO));
            JIF(spMT->SetVideoInfo(&m_pix_fmt,
                &m_width,
                &m_height,
                &m_ratioX,
                &m_ratioY,
                &m_duration));
            JIF(m_spPinIn->SetMediaType(spMT));
        }
    }
    else //break connect
    {

    }
    return hr;
}

STDMETHODIMP CIntelVideoEncoder::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Write(pFrame);
}

HRESULT CIntelVideoEncoder::Open()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
    {
        GetOption(m_spProfile);
        {
            std::cout<<"nTargetUsage\t" << Params.nTargetUsage<< endl;
            std::cout<<"CodecId\t" << Params.CodecId<< endl;
            std::cout<<"ColorFormat\t" << Params.ColorFormat<< endl; // YV12
            std::cout<<"nPicStruct\t" << Params.nPicStruct<< endl;
            std::cout<<"nWidth\t" << Params.nWidth<< endl;
            std::cout<<"nHeight\t" << Params.nHeight<< endl;
            std::cout<<"dFrameRate\t" << Params.dFrameRate<< endl;
            std::cout<<"nNumFrames\t" << Params.nNumFrames<< endl;
            std::cout<<"nBitRate\t" << Params.nBitRate<< endl;
            std::cout<<"MVC_flags\t" << Params.MVC_flags<< endl;
            std::cout<<"nGopPicSize\t" << Params.nGopPicSize<< endl;
            std::cout<<"nGopRefDist\t" << Params.nGopRefDist<< endl;
            std::cout<<"nNumRefFrame\t" << Params.nNumRefFrame<< endl;
            std::cout<<"nBRefType\t" << Params.nBRefType<< endl;
            std::cout<<"nIdrInterval\t" << Params.nIdrInterval<< endl;

            std::cout<<"nQuality\t" << Params.nQuality<< endl;

            std::cout<<"numViews\t" << Params.numViews<< endl;

            std::cout<<"nDstWidth\t" << Params.nDstWidth<< endl;
            std::cout<<"nDstHeight\t" << Params.nDstHeight<< endl;

            std::cout<<"memType\t" << Params.memType<< endl;
            std::cout<<"bUseHWLib\t" << Params.bUseHWLib<< endl;

            cout<<"strSrcFile\t" << (char*)(Params.strSrcFile)<< endl;

            std::cout<<"pluginParams.pluginGuid\t" << Params.pluginParams.pluginGuid.Data<< endl;
            std::cout<<"pluginParams.strPluginPath\t" << Params.pluginParams.strPluginPath<< endl;
            std::cout<<"pluginParams.type\t" << Params.pluginParams.type<< endl;

            std::cout<<".srcFileBuff.size()\t" << Params.srcFileBuff.size()<< endl;
            std::cout<<"dstFileBuff.size()\t" << Params.dstFileBuff.size()<< endl;

            std::cout<<"HEVCPluginVersion\t" << Params.HEVCPluginVersion<< endl;
            std::cout<<"nRotationAngle\t" << Params.nRotationAngle<< endl; // if specified, enables rotation plugin in mfx pipeline
            std::cout<<"strPluginDLLPath\t" << Params.strPluginDLLPath<<endl; // plugin dll path and name

            std::cout<<"nAsyncDepth\t" << Params.nAsyncDepth<< endl; // depth of asynchronous pipeline, this number can be tuned to achieve better performance
            std::cout<<"gpuCopy\t" << Params.gpuCopy<< endl; // GPU Copy mode (three-state option)

            std::cout<<"nRateControlMethod\t" << Params.nRateControlMethod<< endl;
            std::cout<<"nLADepth\t" << Params.nLADepth<< endl; // depth of the look ahead bitrate control  algorithm
            std::cout<<"nMaxSliceSize\t" << Params.nMaxSliceSize<< endl; //maximum size of slice
            std::cout<<"nQPI\t" << Params.nQPI<< endl;
            std::cout<<"nQPP\t" << Params.nQPP<< endl;
            std::cout<<"nQPB\t" << Params.nQPB<< endl;

            std::cout<<"nNumSlice\t" << Params.nNumSlice<< endl;
            std::cout<<"UseRegionEncode\t" << Params.UseRegionEncode<< endl;

            std::cout<<"isV4L2InputEnabled\t" << Params.isV4L2InputEnabled;
        }
        // Choosing which pipeline to use
        pPipeline.reset(CreatePipeline(Params));

        MSDK_CHECK_POINTER(pPipeline.get(), MFX_ERR_MEMORY_ALLOC);
        if (MVC_ENABLED & Params.MVC_flags)
        {
            pPipeline->SetMultiView();
            pPipeline->SetNumView(Params.numViews);
        }

        sts = pPipeline->Init(&Params);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        pPipeline->PrintInfo();

        msdk_printf(MSDK_STRING("Processing started\n"));

        if (pPipeline->CaptureStartV4L2Pipeline() != MFX_ERR_NONE)
        {
            msdk_printf(MSDK_STRING("V4l2 failure terminating the program\n"));
            return 0;
        }
        m_isOpen = true;
    }
    return hr;
}

HRESULT CIntelVideoEncoder::Close()
{
    if(true == m_isOpen)
    {
        //add uninitlize code
        pPipeline->CaptureStopV4L2Pipeline();

        pPipeline->Close();

        m_isOpen = false;
    }
    return S_OK;
}

HRESULT CIntelVideoEncoder::Write(IMediaFrame* pFrame)
{
    HRESULT hr = S_OK;
    if(pFrame != NULL)
    {
        if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
        {
            pPipeline->ResetMFXComponents(&Params); // flash
        }
    }
    auto deliver = [](outter& otr, dom_ptr<IOutputPin>& m_spPinOut, dom_ptr<IMediaFrame>& spFrame)
    {
        HRESULT hr = S_OK;
        spFrame->info.flag = otr.isKeyFrame;
        spFrame->info.dts = otr.dts;
        spFrame->info.pts = otr.timeStamp;
        spFrame->info.samples = otr.used;
        JIF(spFrame->SetBuf(0,otr.used,otr.pData));
        hr = m_spPinOut->Write(spFrame);

        return hr;
    };
    //add encode code
    if (pFrame != NULL)
    {
        IMediaType* pMtIn;
        JCHK(m_spPinIn != NULL,E_FAIL);
        JCHK(pMtIn = m_spPinIn->GetMediaType(),E_FAIL);

        CEncodingPipeline::input data;
        uint8_t* dst_data[4];
        int line[4];
        JIF(pMtIn->FrameToArray(dst_data, line, pFrame));
        int stride = pFrame->info.stride;

        data.y = dst_data[0];
        data.u = dst_data[1];

        data.pts = pFrame->info.pts;
        mfxStatus sts = pPipeline->Run(otr, &data);
        //printf("in : %d\n", pFrame->info.pts);
        if (0 != otr.used)
        {
            //printf("out : pts: %d, dts: %d\n", otr.timeStamp, otr.dts);
            // output
            dom_ptr<IMediaFrame> spFrame;
            JIF(m_spPinOut->AllocFrame(&spFrame));
            deliver(otr, m_spPinOut, spFrame);
            otr.used = 0;
        }
        if (MFX_ERR_NONE <= sts)
            return S_OK;
        // means that the input file has ended, need to go to buffering loops
        MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);
        // exit in case of other errors
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        if (MFX_ERR_DEVICE_LOST == sts || MFX_ERR_DEVICE_FAILED == sts)
        {
            msdk_printf(MSDK_STRING("\nERROR: Hardware device was lost or returned an unexpected error. Recovering...\n"));
            sts = pPipeline->ResetDevice();
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

            sts = pPipeline->ResetMFXComponents(&Params);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
            return S_OK;
        }
        else
        {
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
            return S_FALSE; //?
        }
    }
    else
    {
        if (MFX_ERR_NONE <= pPipeline->Flush(otr)) // flush Encoder
        {
            dom_ptr<IMediaFrame> spFrame;
            JIF(m_spPinOut->AllocFrame(&spFrame));
            return deliver(otr, m_spPinOut,spFrame);

        }
        if (0 != otr.used)
        {
            // output
            dom_ptr<IMediaFrame> spFrame;
            JIF(m_spPinOut->AllocFrame(&spFrame));
            deliver(otr, m_spPinOut,spFrame);
        }
        if (MFX_ERR_NONE == pPipeline->Flush2(otr)) // flush TaskPool
        {
            // out put
            dom_ptr<IMediaFrame> spFrame;
            JIF(m_spPinOut->AllocFrame(&spFrame));
            return deliver(otr, m_spPinOut,spFrame);
        }
        return E_EOF;
    }

    return hr;
}

HRESULT CIntelVideoEncoder::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL == pMtOut)
        return E_INVALIDARG;

    MediaSubType sub = pMtOut->GetSub();
    if(MST_H264 != sub && MST_HEVC != sub)
        return E_INVALIDARG;

    HRESULT hr;

    VideoMediaType pix_fmt_out = VMT_NONE;
    int width_out = 0,height_out = 0;
    int64_t duration_out = 0;

    JIF(pMtOut->GetVideoInfo(&pix_fmt_out,&width_out,&height_out,NULL,NULL,&duration_out));

    if(NULL != pMtIn)
    {

        if(MST_RAWVIDEO != pMtIn->GetSub())
            return E_INVALIDARG;

        VideoMediaType pix_fmt_in = VMT_NONE;
        int width_in = 0,height_in = 0;
        int64_t duration_in = 0;

        JIF(pMtIn->GetVideoInfo(&pix_fmt_in,&width_in,&height_in,NULL,NULL,&duration_in));

        if(VMT_NV12 != pix_fmt_in)
            return E_INVALIDARG;
        if(pix_fmt_in != pix_fmt_out)
            return E_INVALIDARG;
        if(width_in != width_out || height_in != height_out || duration_in != duration_out)
            return E_INVALIDARG;
    }
    return 1;
}

