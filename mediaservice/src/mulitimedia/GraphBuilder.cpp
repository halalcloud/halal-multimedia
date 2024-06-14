#include "GraphBuilder.h"
#include <sys/io.h>
#include <sys/stat.h>

CGraphBuilder::CGraphBuilder()
:m_time(MEDIA_FRAME_NONE_TIMESTAMP)
,m_start(MEDIA_FRAME_NONE_TIMESTAMP)
{
    //ctor
}

bool CGraphBuilder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IGraphBuilder*)this,true,pParam),false);
    JCHK(m_spFG.Create(CLSID_CFilterGraph,(ICallback*)this,false),false);
    return true;
}

bool CGraphBuilder::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear();
        LOG(0,"%s[%p] name:%s release",Class().name,this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CGraphBuilder)
DOM_QUERY_IMPLEMENT(IGraphBuilder)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(const char*) CGraphBuilder::GetName()
{
    return m_stream.c_str();
}

STDMETHODIMP CGraphBuilder::Set(IFilter* pFilter)
{
    JCHK(NULL != pFilter,E_INVALIDARG);
    FilterType type = pFilter->GetType();
    JCHK(FT_Source == type || FT_Render == type,E_INVALIDARG);


    if(false == GUID_CMP(pFilter->Class().clsid,CLSID_CApiSession))
    {
        if(FT_Source == type)
        {
            if(m_spInput != NULL)
            {
                uint32_t status = m_spInput->GetStatus();
                if(status != IFilter::S_Stop)
                {
                    LOG(0,"Push filter:[%s] create graph:[%s] is exist",pFilter->GetName(),m_spInput->GetName());
                    return E_INVALIDARG;
                }
            }
            m_spInput = pFilter;
        }
        else
            m_spOutput = pFilter;
    }
    else
    {
        m_spProfile.QueryFrom(pFilter);
    }
    m_stream = pFilter->GetName();
    return S_OK;
}

STDMETHODIMP CGraphBuilder::Load(const char* pTask)
{
    JCHK(NULL != pTask,E_INVALIDARG);

    HRESULT hr = S_OK;

    try
    {
        Parser parser;

        Dynamic::Var task = parser.parse(pTask);
        Object::Ptr objTask = task.extract<Object::Ptr>();
        JCHK0(false == objTask.isNull(),E_INVALIDARG,"task is not object");

        JCHK(true == objTask->has("output"),E_FAIL);
        JCHK0(true == objTask->isArray("output"),E_INVALIDARG,"task have not output array");
        Dynamic::Var output = objTask->get("output");
        m_outputs = output.extract<Array::Ptr>();
        JCHK0(0 < m_outputs->size(),E_INVALIDARG,"task output array is empty");

        //template
        if(true == objTask->isObject("template"))
        {
            Dynamic::Var tmpl = objTask->get("template");
            Object::Ptr objTempl = tmpl.extract<Object::Ptr>();
            m_template = objTempl->getArray("stream");
            if(false == m_template.isNull())
            {
                for(size_t i=0 ; i<m_template->size() ; ++i)
                {
                    Object::Ptr objMajor;
                    dom_ptr<IMediaType> spMt;
                    Dynamic::Var varStream = m_template->get(i);
                    Object::Ptr objStream = varStream.extract<Object::Ptr>();
                    JCHK0(false == objStream.isNull(),E_INVALIDARG,"task template stream is not object");
                    Object::Ptr objMedia = objStream->getObject("media");
                    JCHK0(false == objMedia.isNull(),E_INVALIDARG,"task template stream have not media object");
                    JCHK(spMt.Create(CLSID_CMediaType),E_FAIL);
                    JIF(Convert(spMt,objMedia));
                    objStream->set("media_type",Dynamic::Var(spMt));
                }
            }
        }

        JCHK(0 < m_outputs->size(),E_INVALIDARG);
        for(size_t i=0 ; i<m_outputs->size() ; ++i)
        {
            Dynamic::Var channel = m_outputs->get(i);
            Object::Ptr objChannel = channel.extract<Object::Ptr>();
            JCHK0(false == objChannel.isNull(),E_INVALIDARG,"task output element not a object");
            JIF(BuildOutput(objChannel));
        }

        if(true == objTask->isObject("input"))
        {
            string strUrl,strFormat,strName;
            Dynamic::Var var = objTask->get("input");
            Object::Ptr input = var.extract<Object::Ptr>();
            JCHK0(false == input.isNull(),E_INVALIDARG,"task have not input object");
            var =  input->get("url_format");
            if(true == var.isString() && false == var.isEmpty())
            {
                strUrl = var.toString();
                GetOutputUrl(strUrl);
            }

            var = input->get("format");
            if(true == var.isString() && false == var.isEmpty())
                strFormat = var.toString();

            var = input->get("name");
            if(true == var.isString() && false == var.isEmpty())
                strName = var.toString();

            if(m_spInput == NULL)
            {
                JIF(m_spFG->Create(FT_Source,strUrl.c_str(),&m_spInput,
                    true == strFormat.empty() ? NULL : strFormat.c_str(),
                    true == strName.empty() ? NULL : strName.c_str()));
            }
            else
            {
                JIF(m_spFG->Append(m_spInput));
            }
            JIF(Convert(m_spInput,input));
            if(false == strUrl.empty())
            {
                dom_ptr<ILoad> spLoad;
                JCHK(m_spInput.Query(&spLoad),E_FAIL);
                JIF(spLoad->Load(NULL,FT_Source));
            }
            else
            {
                JIF(m_spInput->Notify(IFilter::S_Pause));
            }
        }
        m_start = MEDIA_FRAME_NONE_TIMESTAMP;
    }
    catch(...)
    {
        JCHK0(false,E_INVALIDARG,"task is not valid josn string");
        return 0;
    }
    return hr;
}

STDMETHODIMP_(void) CGraphBuilder::Clear()
{
    m_spInputPin = NULL;
    m_spInput = NULL;
    m_spOutput = NULL;
    m_outputs = NULL;
    m_template = NULL;
    m_spFG->Clear();
}

STDMETHODIMP CGraphBuilder::PublishPointAddClient(IOutputPin* pPinOut,IInputPin* pPinIn)
{
    JCHK(NULL != pPinOut,E_INVALIDARG);
    JCHK(NULL != pPinIn,E_INVALIDARG);

    HRESULT hr;
    JIF(pPinOut->Connect(pPinIn));
    uint32_t status = pPinOut->GetFilter()->GetStatus();
    if(status != IFilter::S_Stop)
    {
        IFilter* pFilter;
        JCHK(pFilter = pPinIn->GetFilter(),E_FAIL);
        JIF(pPinIn->Send(IFilter::S_Pause,true,true));
        if(status == IFilter::S_Play)
        {
            JIF(pPinIn->Send(IFilter::S_Play,true,false));
        }
    }
    return hr;
}

HRESULT CGraphBuilder::Play(IFilter* pFilter)
{
    HRESULT hr;
    dom_ptr<IIt> spIt;
    JIF(m_spFG->Enum(FT_Render,&spIt));
    while(spIt->Next())
    {
        dom_ptr<IFilter> spRender;
        JCHK(spRender = m_spFG->Get(spIt),E_FAIL);
        JIF(Connect(pFilter,spRender));
        if(S_FALSE == hr)
            break;
    }
    uint32_t status = pFilter->GetStatus();
    if(IFilter::S_Stop == status)
    {
        JIF(m_spFG->Notify(pFilter,IFilter::S_Pause,true));
    }
    JIF(m_spFG->Notify(pFilter,IFilter::S_Play,false));
    return hr;
}

HRESULT CGraphBuilder::Connect(IFilter* pUpper,IFilter* pDown)
{
    HRESULT hr = S_OK;
    Array* p_streams;
    if(NULL != (p_streams = (Array*)pDown->GetTag()))
    {
        for(size_t i=0 ; i<p_streams->size() ; ++i)
        {
            int index = p_streams->get(i);
            JCHK2(index < (int)m_template->size(),E_INVALIDARG,
                "stream index:%d is out of streams size:%d",
                index,m_template->size());
            Object::Ptr stream = m_template->getObject(index);
            JCHK(false == stream.isNull(),E_INVALIDARG);
            dom_ptr<IMediaType> spMt;
            JCHK(spMt.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMt->CopyFrom(stream->get("media_type").extract< dom_ptr<IMediaType> >()));
            JCHK(spMt != NULL,E_INVALIDARG);
            MediaMajorType major = spMt->GetMajor();
            JCHK(MMT_UNKNOWN < major && MMT_NB > major,E_INVALIDARG);

            dom_ptr<IOutputPin> spPinOut;
            dom_ptr<IInputPin> spPinIn;

            spPinIn = pDown->GetInputPin(i);

            if(spPinIn != NULL)
            {
                if(NULL != (spPinOut = spPinIn->GetConnection()))
                {
                    JIF(Connect(pUpper,spPinOut->GetFilter()));
                    if(S_OK != hr)
                        return hr;
                    continue;
                }
                else
                {
                    JCHK(spMt = spPinIn->GetMediaType(),E_FAIL);
                    if(MMT_DATA == spMt->GetMajor())
                    {
                        uint32_t level = COMPARE_DIFFERENT_VALUE;
                        if(NULL != (spPinOut = Find(pUpper,spMt,NULL,level)))
                        {
                            JIF(spPinOut->Connect(spPinIn));
                            continue;
                        }
                        else
                        {
                            dom_ptr<IFilter> spFilter;
                            JIF(m_spFG->ConnectPin(spPinIn,&spFilter));
                            if(MMT_DATA != major)
                            {
                                pDown->SetTag(NULL);
                                spFilter->SetTag(p_streams);
                            }
                            return Connect(pUpper,spFilter);
                        }
                    }
                }
            }
            dom_ptr<IMediaType> spMtOut;
            if(stream->has("out_put_pin"))
            {
                spPinOut = stream->get("out_put_pin").extract< IOutputPin* >();
                JCHK(spMtOut = spPinOut->GetMediaType(),E_FAIL);
                if(spPinIn == NULL)
                {
                    JCHK(spPinIn = pDown->CreateInputPin(spMtOut),E_FAIL);
                }
                JIF(spPinOut->Connect(spPinIn,spMtOut));
            }
            else
            {
                if(spPinIn == NULL)
                {
                    spPinIn = pDown->CreateInputPin(spMt);
                    if(spPinIn == NULL)
                    {
                        if(MMT_DATA != major)
                        {
                            JIF(spMt->SetMajor(MMT_DATA));
                            JCHK(spPinIn = pDown->CreateInputPin(spMt),E_FAIL);

                            dom_ptr<IFilter> spFilter;
                            JIF(m_spFG->ConnectPin(spPinIn,&spFilter));
                            if(MMT_DATA != major)
                            {
                                pDown->SetTag(NULL);
                                spFilter->SetTag(p_streams);
                            }
                            return Connect(pUpper,spFilter);
                        }
                        else
                        {
                            JCHK2(false,E_FAIL,"can not create [%s:%s] input pin",spMt->GetMajorName(),spMt->GetSubName());
                        }
                    }
                }
                uint32_t level = COMPARE_DIFFERENT_VALUE;
                if(NULL == (spPinOut = Find(pUpper,spMt,NULL,level)))
                {
                    if(NULL != (spPinOut = pUpper->GetOutputPin(0)))
                    {
                        JCHK(spMtOut = spPinOut->GetMediaType(),E_FAIL);
                        if(MMT_DATA == spMtOut->GetMajor())
                        {
                            dom_ptr<IFilter> spFilter;
                            spPinIn = spPinOut->GetConnection();
                            if(spPinIn != NULL)
                            {
                                do
                                {
                                    IFilter* pDemux;
                                    JCHK(pDemux = spPinIn->GetFilter(),E_FAIL);
                                    if(FT_Transform == pDemux->GetType())
                                    {
                                        spFilter = pDemux;
                                        break;
                                    }
                                }while((spPinIn = spPinIn->Next()) != NULL);
                            }
                            if(spFilter == NULL)
                            {
                                JIF(m_spFG->ConnectPin(spPinOut,&spFilter));
                            }
                            return Connect(spFilter,pDown);
                        }
                    }
                    JCHK(spMtOut.Create(CLSID_CMediaType),E_FAIL);
                    JIF(spMtOut->CopyFrom(spMt));
                    if(NULL == (spPinOut = pUpper->CreateOutputPin(spMtOut)))
                        return S_FALSE;
                }
                JCHK(spMtOut = spPinOut->GetMediaType(),E_FAIL);
                JIF(spMt->CopyFrom(spMtOut,COPY_FLAG_COMPILATIONS));

                Array::Ptr filters = stream->getArray(FILTER_TYPE_TRANSFORM);
                if(false == filters.isNull() && 0 < filters->size())
                {
                    dom_ptr<IInputPin> spPin = spPinIn;
                    for(size_t i=0 ; i<filters->size() ; ++i)
                    {
                        Dynamic::Var var = filters->get(i);
                        Object::Ptr filter = var.extract<Object::Ptr>();
                        if(false == filter.isNull())
                        {
                            var = filter->get("name");
                            if(true == var.isString() && false == var.isEmpty())
                            {
                                string name = var;
                                dom_ptr<IFilter> spFilter;
                                JIF(m_spFG->ConnectPin(spPinIn,&spFilter,name.c_str()));
                                JIF(Convert(spFilter,filter));
                                JCHK(spPinIn = spFilter->GetInputPin(0),E_FAIL);
                            }
                        }
                    }
                    JIF(m_spFG->ConnectPin(spPinOut,spPinIn,spPinIn->GetMediaType()));
                    stream->set("out_put_pin",Dynamic::Var(spPin->GetConnection()));
                }
                else
                {
                    JIF(m_spFG->ConnectPin(spPinOut,spPinIn,spMt));
                    if(S_OK == hr)
                        stream->set("out_put_pin",Dynamic::Var(spPinIn->GetConnection()));
                }
            }
        }
    }
    else
    {
        uint32_t count = pDown->GetInputPinCount();
        if(0 < count)
        {
            for(uint32_t i=0 ; i<count ; ++i)
            {
                dom_ptr<IInputPin> spPinIn;
                dom_ptr<IOutputPin> spPinOut;

                JCHK(spPinIn = pDown->GetInputPin(i),E_FAIL);

                dom_ptr<IMediaType> spMtIn;
                JCHK(spMtIn = spPinIn->GetMediaType(),E_FAIL);

                uint32_t level = COMPARE_DIFFERENT_VALUE;
                if(NULL != (spPinOut = Find(pUpper,spMtIn,NULL,level)))
                {
                    dom_ptr<IMediaType> spMtOut = spPinOut->GetMediaType();
                    if(spMtOut != NULL)
                    {
                        JIF(spMtOut->CopyFrom(spMtIn,COPY_FLAG_COMPILATIONS));
                    }
                    else
                        spMtOut = spMtIn;
                    JIF(m_spFG->ConnectPin(spPinOut,spPinIn,spMtOut));
                    if(S_OK != hr)
                        return hr;
                }
                else
                {
                    dom_ptr<IMediaType> spMtOut;
                    spPinOut = pUpper->GetOutputPin(0);
                    if(spPinOut != NULL)
                    {
                        spMtOut = spPinOut->GetMediaType();
                        if(spMtOut != NULL && MMT_DATA == spMtOut->GetMajor())
                        {
                            dom_ptr<IInputPin> spPin = spPinOut->GetConnection();
                            while(spPin != NULL)
                            {
                                dom_ptr<IFilter> spFilter;
                                JCHK(spFilter = spPin->GetFilter(),E_FAIL);
                                if(FT_Transform == spFilter->GetType())
                                    return Connect(spFilter,pDown);
                                else
                                {
                                    spFilter = NULL;
                                    spPin = spPin->Next();
                                }
                            }
                        }
                    }
                    if(MMT_DATA == spMtIn->GetMajor())
                    {
                        dom_ptr<IFilter> spFilter;
                        JIF(m_spFG->ConnectPin(spPinIn,&spFilter));
                        return Connect(pUpper,spFilter);
                    }
                    else if(spMtOut != NULL && MMT_DATA == spMtOut->GetMajor())
                    {
                        dom_ptr<IFilter> spFilter;
                        JIF(m_spFG->ConnectPin(spPinOut,&spFilter));
                        if(0 == spFilter->GetOutputPinCount())
                            return S_FALSE;
                        else
                            return Connect(spFilter,pDown);
                    }
                    else
                        return E_FAIL;
                }
            }
            return hr;
        }
        count = pUpper->GetOutputPinCount();
        if(0 < count)
        {
            for(uint32_t i=0 ; i<count ; ++i)
            {
                dom_ptr<IOutputPin> spPinOut;
                JCHK(spPinOut = pUpper->GetOutputPin(i),E_FAIL);

                dom_ptr<IMediaType> spMtOut;
                dom_ptr<IInputPin> spPinIn;
                JCHK(spMtOut = spPinOut->GetMediaType(),E_FAIL);
                if(NULL != (spPinIn = pDown->CreateInputPin(spMtOut)))
                {
                    JIF(m_spFG->ConnectPin(spPinOut,spPinIn));
                }
                else if(MMT_DATA == spMtOut->GetMajor())
                {
                    dom_ptr<IFilter> spFilter;
                    JIF(m_spFG->ConnectPin(spPinOut,&spFilter));
                    if(0 == spFilter->GetOutputPinCount())
                        return S_FALSE;
                    else
                        JIF(Connect(spFilter,pDown));
                }
                else
                    return E_FAIL;
            }
            return hr;
        }
        else
            return S_FALSE;
    }
    return hr;
}

IOutputPin* CGraphBuilder::Find(IFilter* pFilter,IMediaType* pMT,IOutputPin* pSame,uint32_t& level)
{
    JCHK(NULL != pFilter,NULL);

    uint32_t count = pFilter->GetOutputPinCount();
    for(uint32_t i=0 ; i<count ; ++i)
    {
        dom_ptr<IOutputPin> spPinOut;
        JCHK(spPinOut = pFilter->GetOutputPin(i),NULL);
        dom_ptr<IMediaType> spMT;
        JCHK(spMT = spPinOut->GetMediaType(),NULL);

        uint32_t result = pMT->Compare(spMT);
        if(result < level)
        {
            level = result;
            pSame = spPinOut;
        }

        if(COMPARE_SAME_VALUE == level)
            return pSame;

        dom_ptr<IInputPin> spPinIn;
        if(spPinIn = spPinOut->GetConnection())
        {
            do
            {
                pSame = Find(spPinIn->GetFilter(),pMT,pSame,level);
                if(COMPARE_SAME_VALUE == level)
                    return pSame;
            }while(NULL != (spPinIn = spPinIn->Next()));
        }
    }
    return pSame;
}

HRESULT CGraphBuilder::BuildOutput(Object::Ptr& channel)
{
    HRESULT hr;
    string strUrl,strName,strFormat;
    Dynamic::Var var;

    Array::Ptr p_streams = channel->getArray("stream");

    var = channel->get("url_format");
    if(true == var.isString() && false == var.isEmpty())
    {
        strUrl = var.toString();
        JCHK(GetOutputUrl(strUrl),E_FAIL);
        if(m_spOutput != NULL)
        {
            if(strUrl != m_spOutput->GetName())
            {
                m_spInputPin = m_spOutput->GetInputPin(0);
                if(m_spInputPin != NULL)
                    m_spOutput = NULL;
            }
        }
    }

    string format;
    dom_ptr<IMediaType> spMt;
    Object::Ptr objMedia = channel->getObject("media");
    if(false == objMedia.isNull())
    {
        JCHK(spMt.Create(CLSID_CMediaType),E_FAIL);
        JIF(Convert(spMt,objMedia));
        if(MST_NONE != spMt->GetSub())
            format = spMt->GetSubName();
    }

    var = channel->get("name");
    if(true == var.isString() && false == var.isEmpty())
        strName = var.toString();

    dom_ptr<IFilter> spFilter;
    if(m_spOutput == NULL)
    {
        JIF(m_spFG->Create(FT_Render,strUrl.c_str(),&spFilter,format.c_str(),strName.c_str()));
    }
    else
    {
        JIF(m_spFG->Append(m_spOutput));
        spFilter = m_spOutput;
    }

    JIF(Convert(spFilter,channel));

    if(m_spOutput == NULL)
    {
        dom_ptr<ILoad> spLoad;
        JCHK(spFilter.Query(&spLoad),E_FAIL);
        JIF(spLoad->Load(NULL,FT_Render));
    }

    if(false == objMedia.isNull())
    {
        IInputPin* pPin = spFilter->GetInputPin(0);
        if(NULL == pPin)
        {
            JCHK(spFilter->CreateInputPin(spMt),E_FAIL);
        }
        else
        {
            IMediaType* pMt = pPin->GetMediaType();
            if(NULL != pMt)
            {
                JIF(pMt->CopyFrom(spMt,COPY_FLAG_COMPILATIONS));
            }
            else
            {
                JIF(pPin->SetMediaType(pMt));
            }
        }
    }

    spFilter->SetTag(p_streams.get());
    return hr;
}

STDMETHODIMP CGraphBuilder::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    dom_ptr<IFilter> spFilter;
    JCHK(spFilter.QueryFrom(it->Get()),E_FAIL);

    switch(type)
    {
        case ET_EOF:
        {
            LOG(0,"%s[%p] %s[%p]:[%s] error return%d",this->Class().name,this,spFilter->Class().name,spFilter.p,spFilter->GetName(),param1);
            if(spFilter == m_spInput || spFilter == m_spOutput)
            {
                m_spFG->Notify(m_spInput,IFilter::S_Stop);
                m_spInput = NULL;
                m_spOutput = NULL;
            }
            else
            {
                m_spFG->Notify(spFilter,IFilter::S_Stop);
                return S_OK;
            }
        }
        break;
        case ET_Filter_Build:
        {
            return Play(spFilter);
        }
        break;
        case ET_Filter_Render:
        case ET_Stream_Write:
        {
            if(m_spInput != NULL && m_spOutput != NULL)
            {
                if(0 == (IFilter::FLAG_LIVE & m_spInput->GetFlag()))
                {
                    dom_ptr<ICallback> spCallback;
                    JCHK(m_spInput.Query(&spCallback),E_FAIL);
                    return spCallback->OnEvent((ICallback*)this,NULL,type,param1,param2);
                }
            }
            return type == ET_Filter_Render ? S_OK : E_AGAIN;
        }
        case ET_Filter_Buffer:
            return S_OK;
        case ET_Publish_Add:
        {
            if(m_spInputPin != NULL)
            {
                if(m_stream == spFilter->GetName())
                {
                    HRESULT hr;
                    JIF(PublishPointAddClient((IOutputPin*)param2,m_spInputPin));
                    m_spInputPin = NULL;
                }
            }
        }
        break;
    }
    return m_ep->Notify(type,param1,param2,param3);
}
//
//CGraphBuilder::Stream* CGraphBuilder::Find(uint32_t index,const Object::Ptr& stream)
//{
//	pair<StreamIt,StreamIt> ret = m_streams.equal_range(index);
//	for(StreamIt it = ret.first ; it != ret.second ; ++it)
//	{
//        if(it->second.stream == stream)
//            return &it->second;
//	}
//	return NULL;
//}

HRESULT CGraphBuilder::Convert(IFilter* pFilter,const Object::Ptr& obj)
{
    JCHK(NULL != pFilter,E_INVALIDARG);
    HRESULT hr = S_OK;
    Object::Ptr option = obj->getObject("option");
    if(true != option.isNull())
    {
        dom_ptr<IProfile> spProfile;
        if(true == spProfile.QueryFrom(pFilter))
        {
            JIF(::Convert(spProfile,option));
        }
        JIF(pFilter->Notify(IFilter::C_Profile));
    }
    return hr;
}

HRESULT CGraphBuilder::Convert(IMediaType* pMT,const Object::Ptr& obj)
{
    HRESULT hr;
    Dynamic::Var id = obj->get("id");
    if(true == id.isString())
    {
        string str = id;
        JIF(pMT->SetSub(str.c_str()));
    }
    else
    {
        Dynamic::Var major = obj->get("major");
        JCHK0(true == major.isString(),E_INVALIDARG,"can not get media major type");
        string str = major;
        JIF(pMT->SetMajor(str.c_str()));
    }
    Object::Ptr detail = obj->getObject("option");
    if(false == detail.isNull())
    {
        dom_ptr<IProfile> spProfile;
        JCHK(spProfile.QueryFrom(pMT),E_INVALIDARG);
        JIF(::Convert(spProfile,detail));
    }
    return hr;
}

//IOutputPin* CGraphBuilder::Find(IFilter* pFilter,MediaMajorType major,IMediaType** ppMt,uint32_t id)
//{
//    JCHK(NULL != pFilter,NULL);
//    uint32_t count = pFilter->GetOutputPinCount();
//    for(uint32_t i = 0 ; i < count ; ++i)
//    {
//        dom_ptr<IOutputPin> spPinOut;
//        dom_ptr<IMediaType> spMT;
//        if(NULL != (spMT = spPinOut->GetMediaType()) && id == spPinOut->GetID())
//        {
//            if(spMT->GetMajor() == major)
//            {
//                spMT.CopyTo(ppMt);
//                return spPinOut;
//            }
//        }
//    }
//    return NULL;
//}
//
//IOutputPin* CGraphBuilder::Find(IOutputPin* pPin,bool isCompress)
//{
//    JCHK(NULL != pPin,NULL);
//    dom_ptr<IMediaType> spMT = pPin->GetMediaType();
//    if(spMT == NULL)
//        return NULL;
//    if(isCompress  == spMT->IsCompress())
//    {
//        return pPin;
//    }
//
//    dom_ptr<IInputPin> spPinIn;
//    dom_ptr<IOutputPin> spResult;
//    if(NULL != (spPinIn = pPin->GetConnection()))
//    {
//        do
//        {
//            dom_ptr<IFilter> spFilter;
//            JCHK(spFilter = spPinIn->GetFilter(),NULL);
//            uint32_t count = spFilter->GetOutputPinCount();
//            for(uint32_t i=0 ; i<count ; ++i)
//            {
//                dom_ptr<IOutputPin> spPinOut;
//                JCHK(spPinOut = spFilter->GetOutputPin(i),NULL);
//                spResult = Find(spPinOut,isCompress);
//                if(spResult != NULL)
//                    return spResult;
//            }
//        }while(NULL != (spPinIn = spPinIn->Next()));
//    }
//    return spResult;
//}
//
//HRESULT CGraphBuilder::GetDuration(IProfile* pProfile,int64_t& duration)
//{
//    IProfile::val* pVal = pProfile->Read(VIDEO_DURATION_KEY);
//    if(NULL != pVal)
//    {
//        if(true == STR_CMP(pVal->type,typeid(int).name()))
//            duration = *(int*)pVal->value;
//        else if(true == STR_CMP(pVal->type,typeid(int64_t).name()))
//            duration = *(int64_t*)pVal->value;
//        else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
//            true == STR_CMP(pVal->type,typeid(char*).name()))
//        {
//            char* pUnit = NULL;
//            double val = (double)strtod((const char*)pVal->value, &pUnit);
//            if(0 == strcmp(pUnit,"ms") || 0 == strcmp(pUnit,"MS"))
//                duration = int64_t(val * 10000);
//            else if(*pUnit == 's' || *pUnit == 'S')
//                duration = int64_t(val * 10000000);
//            else if(*pUnit == 'm' || *pUnit == 'M')
//                duration = int64_t(val * 600000000);
//            else if(*pUnit == 'h' || *pUnit == 'H')
//                duration = int64_t(val * 36000000000);
//            else
//                duration = (int64_t)val;
//        }
//        else
//            return S_FALSE;
//    }
//    else
//        return S_FALSE;
//    return S_OK;
//}

bool CGraphBuilder::GetOutputUrl(string& format,const tm* tm,int64_t* timestamp,uint32_t* index)
{
    string result;
    size_t pos = 0,end;
    //time_t long_time = time(NULL);
    //const tm* pTM = localtime(&long_time);

    while(string::npos != (end = format.find_first_of("%%(",pos)))
    {
        result += format.substr(pos,end-pos);
        pos = end + 2;
        if(string::npos == (end = format.find_first_of(')',pos)))
            return false;
        string name = format.substr(pos,end-pos);
        pos = end + 1;
        if(false == m_stream.empty() && name == "stream")
        {
            result += m_stream;
        }
        else if(NULL != tm && name == "date")
        {
            char date[11];
            snprintf(date,11,"%04d-%02d-%02d",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday);
            result += date;
        }
        else if(NULL != tm && name == "time")
        {
            char time[9];
            snprintf(time,9,"%02d-%02d-%02d",tm->tm_hour,tm->tm_min,tm->tm_sec);
            result += time;
        }
        else if(NULL != timestamp && name == "timestamp")
        {
            char time[20];
            snprintf(time,20,"%ld",*timestamp);
            result += time;
        }
        else if(NULL != index && name == "index")
        {
            char str[20];
            snprintf(str,20,"%d",*index);
            result += str;
        }
        else
        {
            result += "%%(";
            result += name;
            result += ")";
        }
    }
    result += format.substr(pos);
    format = result;
    return true;
}

bool CGraphBuilder::CreateDirectory(const string& path)
{
	size_t slash;
    string directory;
	if(string::npos == (slash = path.find_first_of('/')))
		return true;
	while(string::npos != (slash = path.find_first_of('/',slash + 1)))
	{
		directory.assign(path,0,slash+1);
        if(0 != access(directory.c_str(),R_OK))
        {
            if(mkdir(directory.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
                return false;
        }
	}
	return true;
}
//
//HRESULT CGraphBuilder::GetInputInfo(Object::Ptr obj,IFilter* pFilter,HRESULT hr)
//{
//    if(IS_OK(hr))
//    {
//        dom_ptr<IProfile> spProfile;
//        if(NULL != (spProfile.p = static_cast<IProfile*>(pFilter->Query(IID(IProfile)))))
//        {
//            GetInfo(obj,spProfile);
//        }
//        uint32_t count = pFilter->GetOutputPinCount();
//        Array::Ptr streams(new Array());
//        obj->set("streams",streams);
//        for(uint32_t index = 0 ; index < count ; ++index)
//        {
//            dom_ptr<IOutputPin> spPin = pFilter->GetOutputPin(index);
//            if(spPin != NULL)
//            {
//                dom_ptr<IMediaType> spMT;
//                dom_ptr<IProfile> spInfo;
//                JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
//                JIF(spPin->GetMediaType(spMT));
//                Object::Ptr stream(new Object());
//                Object::Ptr info(new Object());
//                stream->set(spMT->GetMajorName(),info);
//                info->set("id",spMT->GetSubName());
//                if(true == spMT.Query(&spInfo))
//                    GetInfo(info,spInfo);
//                streams->add(stream);
//            }
//        }
//    }
//    else
//    {
//        obj->set("result",hr);
//        obj->set("descr","load url fail");
//    }
//    return hr;
//}
//
//void CGraphBuilder::GetInfo(Object::Ptr obj,IProfile* pProfile)
//{
//    IProfile::val* pVal;
//    IProfile::Name name = NULL;
//    while(NULL != (name = pProfile->Next(name)))
//    {
//        if(NULL != (pVal = pProfile->Read(name)) && NULL != (pVal->value))
//        {
//            if(true == STR_CMP(pVal->type,typeid(int).name()))
//                obj->set(name,*(int*)pVal->value);
//            else if(true == STR_CMP(pVal->type,typeid(int64_t).name()))
//                obj->set(name,*(int64_t*)pVal->value);
//            else if(true == STR_CMP(pVal->type,typeid(float).name()))
//                obj->set(name,*(float*)pVal->value);
//            else if(true == STR_CMP(pVal->type,typeid(double).name()))
//                obj->set(name,*(double*)pVal->value);
//            else if(true == STR_CMP(pVal->type,typeid(char*).name())||
//                true == STR_CMP(pVal->type,typeid(const char*).name()))
//                obj->set(name,(const char*)pVal->value);
//        }
//    }
//}
