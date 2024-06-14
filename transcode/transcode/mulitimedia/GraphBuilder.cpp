#include "GraphBuilder.h"
#include <sys/io.h>
#include <sys/stat.h>

CGraphBuilder::CGraphBuilder()
:m_callback(NULL)
{
    //ctor
    m_status.isLive = false;
    m_status.isCycle = false;
    m_status.number = 0;
    m_status.count = 0;
    m_status.progress = 0.0;
    m_status.speed = 1.0;
    m_status.clock = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.time = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.buffer = 0;
    m_duration = MEDIA_FRAME_NONE_TIMESTAMP;
    m_begin = MEDIA_FRAME_NONE_TIMESTAMP;
    g_pSite->DumpSetCallback(dump_callback,this);
}

bool CGraphBuilder::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_spFG.Create(CLSID_CFilterGraph),false);
    m_spFG->SetEvent(dynamic_cast<IFilterEvent*>(this));
    return true;
}

bool CGraphBuilder::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        LOG(0,"graph builder release stop");
        Stop();
        g_pSite->DumpSetCallback(NULL,NULL);
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CGraphBuilder)
DOM_QUERY_IMPLEMENT(IGraphBuilder)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CGraphBuilder::Play(const char* pTask,bool isWait,IGraphBuilderCallback* pCallback)
{
    JCHK(NULL != pTask,E_INVALIDARG);
    m_callback = pCallback;
    m_status.isLive = false;
    m_status.isCycle = false;
    m_status.number = 0;
    m_status.count = 0;
    m_status.progress = 0.0;
    m_status.speed = 1.0;
    m_status.clock = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.time = MEDIA_FRAME_NONE_TIMESTAMP;
    m_status.buffer = 0;
    m_duration = MEDIA_FRAME_NONE_TIMESTAMP;
    m_begin = MEDIA_FRAME_NONE_TIMESTAMP;
    HRESULT hr = S_OK;
    try
    {
        Parser parser;

        Dynamic::Var task = parser.parse(pTask);
        Object::Ptr objTask = task.extract<Object::Ptr>();
        JCHK0(false == objTask.isNull(),E_INVALIDARG,"task is not object");

        JCHK0(true == objTask->isObject("input"),E_INVALIDARG,"task have not input object");
        Dynamic::Var input = objTask->get("input");
        Object::Ptr objInput = input.extract<Object::Ptr>();

        //input
        Dynamic::Var cycle = objInput->get("cycle");
        if(true == cycle.isBoolean())
            m_status.isCycle = cycle;

        JCHK0(true == objInput->isArray("source"),E_INVALIDARG,"task input have not source array");
        Dynamic::Var source = objInput->get("source");
        m_inputs = source.extract<Array::Ptr>();
        m_itInput = m_inputs->end();

        if(true == objTask->has("output"))
        {
            JCHK0(true == objTask->isObject("template"),E_INVALIDARG,"task have not template object");
            Dynamic::Var tmpl = objTask->get("template");
            Object::Ptr objTempl = tmpl.extract<Object::Ptr>();

            JCHK0(true == objTask->isArray("output"),E_INVALIDARG,"task have not output array");
            Dynamic::Var output = objTask->get("output");
            m_outputs = output.extract<Array::Ptr>();

            JCHK0(0 < m_outputs->size(),E_INVALIDARG,"task output array is empty");
            //template
            Array::Ptr streams = objTempl->getArray("stream");
            JCHK0(false == streams.isNull(),E_INVALIDARG,"task template have not stream array");
            JCHK0(0 < streams->size(),E_INVALIDARG,"task template stream array is empty");

            for(size_t i=0 ; i<streams->size() ; ++i)
            {
                Object::Ptr objMajor;
                dom_ptr<IMediaType> spMt;
                Dynamic::Var varStream = streams->get(i);
                Object::Ptr objStream = varStream.extract<Object::Ptr>();
                JCHK0(false == objStream.isNull(),E_INVALIDARG,"task template stream is not object");
                Object::Ptr objMedia = objStream->getObject("media");
                JCHK0(false == objMedia.isNull(),E_INVALIDARG,"task template stream have not media object");
                JCHK(spMt.Create(CLSID_CMediaType),E_FAIL);
                JIF(Convert(spMt,objMedia));
                objStream->set("media_type",Dynamic::Var(spMt));
            }

            JCHK0(S_OK == Next(S_STREAM_EOF),E_INVALIDARG,"task input have not valid source")
            //output
            for(size_t i=0 ; i<m_outputs->size() ; ++i)
            {
                Dynamic::Var channel = m_outputs->get(i);
                Object::Ptr objChannel = channel.extract<Object::Ptr>();
                JCHK0(false == objChannel.isNull(),E_INVALIDARG,"task output element not a object");
                JIF(AddOutput(streams,objChannel));
            }
            m_status.count = m_inputs->size();
            SetInputEnd();
            JIF(m_spFG->Play(isWait));
        }
        else
        {
            while(S_STREAM_EOF != Next(S_STREAM_EOF))
                SetInputEnd();
            if(NULL != m_callback)
                m_callback->OnEnd(S_OK,GetInfo(it_input));
        }
    }
    catch(...)
    {
        JCHK0(false,E_INVALIDARG,"task is not valid josn string");
        return 0;
    }
    return hr;
}

STDMETHODIMP CGraphBuilder::Stop()
{
    LOG(0,"graph builder stop");
    HRESULT hr;
    JIF(m_spFG->Clear());
    m_renders.clear();
    return hr;
}

STDMETHODIMP_(bool) CGraphBuilder::IsRunning()
{
    return m_spFG->IsRunning();
}

STDMETHODIMP_(const char*) CGraphBuilder::GetInfo(info_type it)
{
    Object msg;
    if(IGraphBuilder::it_input == it)
    {
        msg.set("msg","input");
        msg.set("input",m_inputs);
    }
    else if(IGraphBuilder::it_status == it)
    {
        msg.set("msg","status");
        Object status;
        const Status& s = GetStatus();
        status.set("is_running",s.isRun);
        status.set("is_live",s.isLive);
        status.set("is_cycle",s.isCycle);
        status.set("input_number",s.number);
        status.set("input_count",s.count);
        status.set("progress_percent",s.progress);
        status.set("speed_x",s.speed);
        status.set("buffer_ms",s.buffer/10000);
        double expend_max = 0.0;
        string io_name;
        if(m_spInput != NULL)
        {
            Object input;
            double expend = m_spInput->GetExpend();
            expend = int(expend * 100)/100.0;
            input.set(m_spInput->GetName(),expend);
            status.set("input_expend_percent",input);
            if(expend > expend_max)
            {
                expend_max = expend;
                io_name = "input:";
                io_name += m_spInput->GetName();
            }
        }
        if(false == m_renders.empty())
        {
            Object output;
            for(FilterIt it=m_renders.begin() ; it!=m_renders.end() ; ++it)
            {
                double expend = it->p->GetExpend();
                expend = int(expend * 100)/100.0;
                output.set(it->p->GetName(),expend);
                if(expend > expend_max)
                {
                    expend_max = expend;
                    io_name = "output:";
                    io_name += it->p->GetName();
                }
            }
            status.set("output_expend_percent",output);
        }
        msg.set("status",status);
        if(true == s.isLive)
        {
            if(s.speed <= 0.9)
            {
                LOG(1,"live transcode speed:%f.2 too slow,%s expend is %f.2",s.speed,io_name.c_str(),expend_max);
                if(s.speed == 0.0)
                {

                }
            }
        }
    }
    else
        return NULL;
    Dynamic::Var var = msg;
    m_info[it] = var.toString();
    return m_info[it].c_str();
}

const Status& CGraphBuilder::GetStatus()
{
    m_status.isRun = m_spFG != NULL &&  true == m_spFG->IsRunning();
    const filter_graph_status& status = m_spFG->GetStatus();

    if(MEDIA_FRAME_NONE_TIMESTAMP != status.timeInput && MEDIA_FRAME_NONE_TIMESTAMP != status.timeOutput)
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP != m_begin && MEDIA_FRAME_NONE_TIMESTAMP != m_duration)
            m_status.progress = (status.timeInput - m_begin) * 100.0 / m_duration;
        else
            m_status.progress = 0;

        m_status.progress = ((m_status.number - 1) * 100.0 + m_status.progress) / m_status.count;
        m_status.progress = int(m_status.progress * 100) / 100.0;
        int64_t clock = status.clockTime - status.clockStart;
        int64_t time = status.timeOutput - status.timeStart;

        if(MEDIA_FRAME_NONE_TIMESTAMP == m_status.time)
            m_status.speed = 1.0;
        else if(clock == m_status.clock || time == m_status.time)
            m_status.speed = 0.0;
        else
            m_status.speed = double(time - m_status.time)/(clock - m_status.clock);
        m_status.speed = int(m_status.speed * 100) / 100.0;
        m_status.clock = clock;
        m_status.time = time;
        m_status.buffer = status.timeInput - status.timeOutput;
    }
    return m_status;
}

HRESULT CGraphBuilder::Next(HRESULT hr)
{
    HRESULT rl;
    Array::ValueVec::const_iterator it = m_itInput;
    do
    {
        if(m_inputs->end() == it)
        {
            it = m_inputs->begin();
            m_status.number = 0;
        }
        else
            ++it;
        if(it == m_inputs->end())
        {
            if(false == m_status.isCycle)
                return hr;
            it = m_inputs->begin();
            m_status.number = 1;
        }
        else
            ++m_status.number;
    }while(S_OK > (rl = SetInputBeg(it)));
    return rl;
}

HRESULT CGraphBuilder::SetInputBeg(Array::ValueVec::const_iterator& it)
{
    HRESULT hr;
    Object::Ptr obj = it->extract<Object::Ptr>();
    JCHK0(false == obj.isNull(),E_INVALIDARG,"input is not object");

    Dynamic::Var var;
    string strUrl,strName;
    var =  obj->get("url");
    JCHK0(true == var.isString() && false == var.isEmpty(),E_INVALIDARG,"input url is not string");
    strUrl = var.toString();
    var = obj->get("name");
    if(true == var.isString() && false == var.isEmpty())
        strName = var.toString();

    dom_ptr<IFilter> spFilter;
    hr = m_spFG->LoadSource(strUrl.c_str(),&spFilter,true == strName.empty() ? NULL : strName.c_str());
    GetInputInfo(obj,spFilter,hr);
    if(IS_OK(hr))
    {
        JIF(Convert(spFilter,obj));
        JIF(SetInput(spFilter));
        m_itNewInput = it;
    }
    return hr;
}

HRESULT CGraphBuilder::SetInputEnd()
{
    if(m_itNewInput == m_inputs->end())
        return S_FALSE;
    m_itInput = m_itNewInput;
    m_itNewInput = m_inputs->end();
    m_status.progress = 0.0;
    return S_OK;
}

HRESULT CGraphBuilder::SetInput(IFilter* pFilter)
{
    JCHK(NULL != pFilter,E_INVALIDARG);
    HRESULT hr = S_OK;
    m_duration = MEDIA_FRAME_NONE_TIMESTAMP;
    if(m_spInput != NULL)
    {
        if(m_spInput != pFilter)
        {
            uint32_t sets[MMT_NB][2];
            memset(sets,0,sizeof(sets));
            dom_ptr<IDemuxer> spDemuxer;
            JCHK(spDemuxer.p = static_cast<IDemuxer*>(pFilter->Query(IID(IDemuxer))),E_FAIL);
            for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
            {
                dom_ptr<IMediaType> spMt = it->second.stream->get("media_type").extract< dom_ptr<IMediaType> >();
                if(spMt != NULL)
                {
                    MediaMajorType major = spMt->GetMajor();
                    if(MMT_UNKNOWN < major && MMT_NB > major)
                    {
                        if(0 == sets[major][0])
                            sets[major][0] = it->first + 1;
                        else if(sets[major][0] - 1 != it->first)
                            ++sets[major][1];
                        it->second.spPin = Find(pFilter,major,NULL,sets[major][1]);
                        if(it->second.spPin == NULL)
                        {
                            JCHK2(it->second.spPin = spDemuxer->CreatePin(spMt),E_INVALIDARG,
                                "source %s can not find or create %s stream",
                                pFilter->GetName(),spMt->GetMajorName());
                        }
                        sets[major][1] = it->second.spPin->GetIndex();
                    }
                }
            }
            m_spNewInput = pFilter;
        }
    }
    else
    {
        dom_ptr<IDemuxer> spDemuxer;
        int64_t time = 0;
        JCHK(spDemuxer.p = static_cast<IDemuxer*>(pFilter->Query(IID(IDemuxer))),E_FAIL);
        spDemuxer->SetStartTime(time);
        m_spInput = pFilter;
        m_begin = time;
    }

    dom_ptr<IProfile> spProfile;
    m_duration = MEDIA_FRAME_NONE_TIMESTAMP;
    if(NULL != (spProfile.p = static_cast<IProfile*>(pFilter->Query(IID(IProfile)))))
        spProfile->Read(STREAM_LENGTH_KEY,m_duration);
    int64_t duration = m_duration;
    GetDuration(spProfile,duration);
    if((0 < duration && MEDIA_FRAME_NONE_TIMESTAMP == m_duration) || (0 < m_duration && 0 < duration && duration < m_duration))
        m_duration = duration;
    return hr;
}

HRESULT CGraphBuilder::AddOutput(const Array::Ptr& streams,Object::Ptr& channel)
{
    JCHK(m_spInput != NULL,E_FAIL);

    HRESULT hr = S_OK;
    uint32_t sets[MMT_NB];
    dom_ptr<IFilter> spFilter;
    dom_ptr<IMuxer> spMuxer;
    memset(sets,0,sizeof(sets));
    Array::Ptr p_streams = channel->getArray("stream");
    JCHK(false == p_streams.isNull(),E_INVALIDARG);
    JCHK(0 < p_streams->size(),E_INVALIDARG);

    string strUrl,strName;
    Dynamic::Var var;
    var = channel->get("url_format");
    JCHK(true == var.isString() && false == var.isEmpty(),E_INVALIDARG);
    strUrl = var.toString();
    var = channel->get("name");
    if(true == var.isString() && false == var.isEmpty())
        strName = var.toString();
    JIF(m_spFG->LoadRender(strUrl.c_str(),&spFilter,true == strName.empty() ? NULL : strName.c_str()));

    JCHK(spFilter.Query(&spMuxer),E_FAIL);
    for(size_t i=0 ; i<p_streams->size() ; ++i)
    {
        int index = p_streams->get(i);
        JCHK2(index < (int)streams->size(),E_INVALIDARG,
            "stream index:%d is out of streams size:%d",
            index,streams->size());
        Object::Ptr stream = streams->getObject(index);
        JCHK(false == stream.isNull(),E_INVALIDARG);
        dom_ptr<IMediaType> spMt = stream->get("media_type").extract< dom_ptr<IMediaType> >();
        JCHK(spMt != NULL,E_INVALIDARG);
        MediaMajorType major = spMt->GetMajor();
        JCHK(MMT_UNKNOWN < major && MMT_NB > major,E_INVALIDARG);
        dom_ptr<IMediaType> spMtOut,spMtTemp;
        JCHK(spMtTemp.Create(CLSID_CMediaType),E_FAIL);
        dom_ptr<IOutputPin> spPinOut = Find(m_spInput,major,&spMtOut,sets[major]);
        if(spPinOut == NULL)
        {
            dom_ptr<IDemuxer> spDemuxer;
            JCHK(m_spInput.Query(&spDemuxer),E_INVALIDARG);
            JCHK(spMtOut.Create(CLSID_CMediaType),E_FAIL);
            JIF(spMtOut->CopyFrom(spMt));
            if((spPinOut = spDemuxer->CreatePin(spMtOut)) == NULL)
            {
                LOG(1,"output url:%s not support %s stream",strUrl.c_str(),spMtOut->GetSubName());
                continue;
            }

        }
        if(spPinOut != NULL)
        {
            JIF(spMtTemp->CopyFrom(spMtOut));
            JIF(spMtTemp->CopyFrom(spMt,COPY_FLAG_COMPILATIONS | COPY_FLAG_INTERSECTION));


            dom_ptr<IInputPin> spPinIn;
            sets[major] = spPinOut->GetIndex() + 1;


            JCHK(spPinIn = spMuxer->CreatePin(spMtTemp),E_FAIL);
            Stream* pStrm = Find(spPinOut->GetIndex(),stream);
            if(NULL != pStrm)
            {
                JCHK(pStrm->spPin != NULL,E_FAIL);
                JCHK1(S_OK == pStrm->spPin->Connect(spPinIn,NULL),E_FAIL,"output url:%s connect pin fail",strUrl.c_str());
                JIF(spPinIn->SetTag((void*)pStrm));
            }
            else
            {
                Stream strm(stream);
                dom_ptr<IInputPin> spPin = spPinIn;
                Array::Ptr filters = stream->getArray(FILTER_TYPE_TRANSFORM);
                if(false == filters.isNull())
                {
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
                                JIF(m_spFG->ConnectPin(spPin,spMtTemp,name.c_str(),&spFilter));
                                JCHK(spPin = spFilter->GetInputPin(0),E_FAIL);
                                JIF(Convert(spFilter,filter));
                                spMtTemp = NULL;
                            }
                        }
                    }
                }
                if(spMtTemp != NULL && S_OK == spMtTemp->Compare(spMtOut))
                {
                    JCHK1(S_OK == spPinOut->Connect(spPin,NULL),E_FAIL,"output url:%s connect pin fail",strUrl.c_str());
                }
                else
                {
                    dom_ptr<IOutputPin> spPinOutDecoder = Find(spPinOut,false);
                    if(spPinOutDecoder != NULL)
                    {
                        JCHK1(S_OK == m_spFG->ConnectPin(spPinOutDecoder,spPin,spMtTemp),E_FAIL,"output url:%s connect pin fail",strUrl.c_str());
                    }
                    else
                    {
                        JCHK1(S_OK == m_spFG->ConnectPin(spPinOut,spPin,spMtTemp),E_FAIL,"output url:%s connect pin fail",strUrl.c_str());
                    }
                }
                strm.spPin = spPinIn->GetConnection();
                if(spPinIn == spPin && true == spMtTemp->IsCompress())
                {
                    dom_ptr<IFilter> spFilter = strm.spPin->GetFilter();
                    if(STR_CMP(spFilter->Class().pType,FILTER_TYPE_TRANSFORM))
                    {
                        JCHK(spPin = strm.spPin->GetFilter()->GetInputPin(0),E_FAIL);
                    }
                    else
                        spPin = spPinIn;
                }
                JIF(spPin->SetTag((void*)&m_streams.insert(StreamPair(spPinOut->GetIndex(),strm))->second));
            }
        }
    }

    JIF(spFilter->SetTag((void*)channel.get()));
     JIF(Convert(spFilter,channel));
    m_renders.push_back(spFilter);
    return hr;
}

STDMETHODIMP CGraphBuilder::OnEvent(EventType type,HRESULT hr,IFilter* pFilter,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame,void* pTag)
{
    if(IFilterEvent::Open == type)
    {
        if(m_spInput.p == pFilter)
        {
            const filter_graph_status& status = m_spFG->GetStatus();
            if(MEDIA_FRAME_NONE_TIMESTAMP != status.timeInput)
            {
                dom_ptr<IDemuxer> spDemuxer;
                JCHK(spDemuxer.p = static_cast<IDemuxer*>(pFilter->Query(IID(IDemuxer))),E_FAIL);
                int64_t time = status.timeInput;
                if(true == m_status.isLive)
                {
                    if(MEDIA_FRAME_NONE_TIMESTAMP != status.timeStart)
                    {
                        int64_t clock = GetTickCount() - status.clockStart;
                        int64_t input = status.timeInput - status.timeStart;
                        //int64_t buffer = status.timeInput - status.timeOutput;
                        int64_t buffer = 10000000;
                        if(clock > input)
                        {
                            spDemuxer->NewSegment();
                            time = clock + status.timeStart + buffer;
                            LOG(1,"input time:%ldms adjust to clock time:%ldms delta:%ldms",
                                int(status.timeInput/10000),int(time/10000),(time - status.timeInput)/10000);
                        }
                    }
                }
                spDemuxer->SetStartTime(time);
                m_begin = time;
            }
        }
        else if(STR_CMP(pFilter->Class().pType,FILTER_TYPE_RENDER))
        {
            Object* obj = (Object*)pFilter->GetTag();
            if(NULL != obj)
            {
                Object::Ptr result = obj->getObject("results");
                if(true == result.isNull())
                {
                    result = new Object();
                    obj->set("results",result);
                }
                result->set(pFilter->GetName(),"open");
            }
            if(NULL != m_callback)
            {
                Object msg;
                msg.set("msg","output_open");
                Object output_open;
                output_open.set("url",pFilter->GetName());
                msg.set("output_open",output_open);
                Dynamic::Var var = msg;
                string str = var.toString();
                m_callback->OnMsg(str.c_str());
            }
            if(0 != (FILTER_FLAG_LIVE & pFilter->GetFlag()))
                m_status.isLive = true;
        }
    }
    else if(IFilterEvent::Process == type)
    {
        if(NULL != pFilter)
        {
            if(NULL != pFrame)
            {
                if(STR_CMP(pFilter->Class().pType,FILTER_TYPE_RENDER) && true == pFrame->info.segment)
                {
                    int64_t duration = 0;
                    dom_ptr<IProfile> spProfile;
                    JCHK(spProfile.p = static_cast<IProfile*>(pFilter->Query(IID(IProfile))),E_FAIL);
                    GetDuration(spProfile,duration);
                    Object* obj = (Object*)pFilter->GetTag();
                    if(0 < duration && NULL != obj)
                    {
                        Dynamic::Var var = obj->get("url_format");
                        if(true == var.isString() && false == var.isEmpty())
                        {
                            string strUrl = var;
                            int64_t start = MEDIA_FRAME_NONE_TIMESTAMP;
                            uint32_t index = 0;
                            spProfile->Read("start",start);
                            spProfile->Read("index",index);

                            if(MEDIA_FRAME_NONE_TIMESTAMP == start || (0 < duration && pFrame->info.dts - start >= duration))
                            {
                                JCHK1(true == GetOutputUrl(strUrl,pFrame->info.dts,index),E_INVALIDARG,"invalid url_format:%s",strUrl.c_str());
                                if(string::npos == strUrl.find("://"))
                                {
                                    JCHK1(true == CreateDirectory(strUrl),E_FAIL,"create directory:%s fail",strUrl.c_str());
                                }
                                if(MEDIA_FRAME_NONE_TIMESTAMP != start)
                                    pFilter->Close();
                                JIF(pFilter->SetName(strUrl.c_str()));
                                start = pFrame->info.dts;
                                ++index;
                                JCHK(spProfile->Write("start",start),E_FAIL);
                                JCHK(spProfile->Write("index",index),E_FAIL);
                            }
                        }
                    }
                    else
                    {
                        int32_t index = 0;
                        string strUrl = pFilter->GetName();
                        JCHK1(true == GetOutputUrl(strUrl,pFrame->info.dts,index),E_INVALIDARG,"invalid url_format:%s",strUrl.c_str());
                        if(string::npos == strUrl.find("://"))
                        {
                            JCHK1(true == CreateDirectory(strUrl),E_FAIL,"create directory:%s fail",strUrl.c_str());
                        }
                        JIF(pFilter->SetName(strUrl.c_str()));
                    }
                }
                else if(m_spInput == pFilter && NULL != pFrame)
                {
                    if(0 <= m_duration)
                    {
                        if(MEDIA_FRAME_NONE_TIMESTAMP != pFrame->info.dts && pFrame->info.dts - m_begin >= m_duration)
                            hr = S_STREAM_EOF;
                    }
                }
            }
            else if(NULL != pPinIn && NULL != pPinIn->GetTag())
            {
                if(m_itNewInput != m_inputs->end())
                {
                    if(m_spNewInput != NULL)
                    {
                        dom_ptr<IMediaType> spMt;
                        Stream* pStream = (Stream*)pPinIn->GetTag();
                        JIF(spMt.Create(CLSID_CMediaType));
                        JIF(pPinIn->GetMediaType(spMt));
                        pPinIn->Disconnect();
                        JIF(m_spFG->ConnectPin(pStream->spPin,pPinIn,spMt));
                        JCHK(pStream->spPin = pPinIn->GetConnection(),E_FAIL);
                    }
                    hr = S_STREAM_EOF;
                }
            }
        }
    }
    else if(IFilterEvent::Close == type)
    {
        if(NULL != pFilter)
        {
            if(STR_CMP(pFilter->Class().pType,FILTER_TYPE_RENDER))
            {
                Object* obj = (Object*)pFilter->GetTag();
                if(NULL != obj)
                {
                    Object::Ptr result = obj->getObject("results");
                    if(false == result.isNull())
                        result->set(pFilter->GetName(),"close");
                }
                if(NULL != m_callback)
                {
                    Object msg;
                    msg.set("msg","output_close");
                    Object output_close;
                    output_close.set("url",pFilter->GetName());
                    msg.set("output_close",output_close);
                    Dynamic::Var var = msg;
                    string str = var.toString();
                    m_callback->OnMsg(str.c_str());
                }
            }
        }
    }
    else if(IFilterEvent::FlushBegin == type)
    {
        if(m_spInput == pFilter)
        {
            return true == m_spFG->IsExit() ||S_OK == Next(hr) ? S_STREAM_EOF : hr;
        }
    }
    else if(IFilterEvent::FlushEnd == type)
    {
        if(m_spInput == pFilter)
        {
            if(NULL != m_spNewInput)
            {
                if(m_spInput != NULL)
                    Remove(m_spInput);
                m_spInput = m_spNewInput;
                m_spNewInput = NULL;
            }
            if(S_OK == SetInputEnd())
                hr = S_OK;
        }
    }
    else if(IFilterEvent::End == type)
    {
        LOG(0,"graph builder recv end event hr:%d",hr);
        if(NULL != m_callback)
        {
            if(S_OK <= hr)
            {
                if(true == m_outputs.isNull())
                {
                    m_callback->OnEnd(hr,GetInfo(it_input));
                }
                else
                {
                    Object msg;
                    msg.set("msg","succees");
                    msg.set("succees",m_outputs);
                    Dynamic::Var var = msg;
                    string str = var.toString();
                    m_callback->OnEnd(hr,str.c_str());
                }
            }
            else
            {
                m_callback->OnEnd(hr,m_msg_error.c_str());
            }
        }
    }
    return hr;
}

CGraphBuilder::Stream* CGraphBuilder::Find(uint32_t index,const Object::Ptr& stream)
{
	pair<StreamIt,StreamIt> ret = m_streams.equal_range(index);
	for(StreamIt it = ret.first ; it != ret.second ; ++it)
	{
        if(it->second.stream == stream)
            return &it->second;
	}
	return NULL;
}

HRESULT CGraphBuilder::Convert(IFilter* pFilter,const Object::Ptr& obj)
{
    HRESULT hr = S_OK;
    Object::Ptr option = obj->getObject("option");
    if(true != option.isNull())
    {
        dom_ptr<IProfile> spProfile;
        if(NULL != (spProfile.p = static_cast<IProfile*>(pFilter->Query(IID(IProfile)))))
        {
            JIF(Convert(spProfile,option));
        }
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
        JCHK(spProfile.p = static_cast<IProfile*>(pMT->Query(IID(IProfile))),E_INVALIDARG);
        JIF(Convert(spProfile,detail));
    }
    return hr;
}

HRESULT CGraphBuilder::Convert(IProfile* pProfile,const Object::Ptr& obj)
{
    HRESULT hr = S_OK;
    for(Object::ConstIterator it=obj->begin() ; it!=obj->end() ; ++it)
    {
        if(true == obj->isObject(it))
        {
            Object::Ptr objTemp = it->second.extract<Object::Ptr>();
            if(false == objTemp.isNull())
            {
                IProfile::val* pVal = pProfile->Write(it->first.c_str(),IID(IProfile),NULL,0);
                if(NULL == pVal || NULL == pVal->value)
                    return E_INVALIDARG;
                JIF(Convert((IProfile*)pVal->value,objTemp));
            }
        }
        else
        {
            if(it->second.isString())
            {
                string val = it->second;
                pProfile->Write(it->first.c_str(),val.c_str(),val.size()+1);
            }
            else if(it->second.isInteger())
            {
                int val = it->second;
                pProfile->Write(it->first.c_str(),val);
            }
            else if(true == it->second.isNumeric())
            {
                double val = it->second;
                pProfile->Write(it->first.c_str(),val);
            }
            else
            {
                LOG(0,"template stream: unknown type",it->second.toString().c_str());
            }
        }
    }
    return hr;
}

IOutputPin* CGraphBuilder::Find(IFilter* pFilter,MediaMajorType major,IMediaType** ppMt,uint32_t index)
{
    JCHK(NULL != pFilter,NULL);
    uint32_t count = pFilter->GetOutputPinCount();
    for(uint32_t i = index ; i < count ; ++i)
    {
        dom_ptr<IOutputPin> spPinOut;
        dom_ptr<IMediaType> spMT;
        JCHK(spPinOut = pFilter->GetOutputPin(i),NULL);
        JCHK(spMT.Create(CLSID_CMediaType),NULL);
        JCHK(S_OK == spPinOut->GetMediaType(spMT),NULL);
        if(spMT->GetMajor() == major)
        {
            if(NULL != ppMt)
                spMT.CopyTo(ppMt);
            return spPinOut;
        }
    }
    return NULL;
}

IOutputPin* CGraphBuilder::Find(IOutputPin* pPin,bool isCompress)
{
    JCHK(NULL != pPin,NULL);
    dom_ptr<IMediaType> spMT = pPin->GetMediaType();
    if(spMT == NULL)
        return NULL;
    if(isCompress  == spMT->IsCompress())
    {
        return pPin;
    }

    dom_ptr<IInputPin> spPinIn;
    dom_ptr<IOutputPin> spResult;
    if(NULL != (spPinIn = pPin->GetConnection()))
    {
        do
        {
            dom_ptr<IFilter> spFilter;
            JCHK(spFilter = spPinIn->GetFilter(),NULL);
            uint32_t count = spFilter->GetOutputPinCount();
            for(uint32_t i=0 ; i<count ; ++i)
            {
                dom_ptr<IOutputPin> spPinOut;
                JCHK(spPinOut = spFilter->GetOutputPin(i),NULL);
                spResult = Find(spPinOut,isCompress);
                if(spResult != NULL)
                    return spResult;
            }
        }while(NULL != (spPinIn = spPinIn->Next()));
    }
    return spResult;
}

HRESULT CGraphBuilder::GetDuration(IProfile* pProfile,int64_t& duration)
{
    IProfile::val* pVal = pProfile->Read(VIDEO_DURATION_KEY);
    if(NULL != pVal)
    {
        if(true == STR_CMP(pVal->type,typeid(int).name()))
            duration = *(int*)pVal->value;
        else if(true == STR_CMP(pVal->type,typeid(int64_t).name()))
            duration = *(int64_t*)pVal->value;
        else if(true == STR_CMP(pVal->type,typeid(const char*).name())||
            true == STR_CMP(pVal->type,typeid(char*).name()))
        {
            char* pUnit = NULL;
            double val = (double)strtod((const char*)pVal->value, &pUnit);
            if(0 == strcmp(pUnit,"ms") || 0 == strcmp(pUnit,"MS"))
                duration = int64_t(val * 10000);
            else if(*pUnit == 's' || *pUnit == 'S')
                duration = int64_t(val * 10000000);
            else if(*pUnit == 'm' || *pUnit == 'M')
                duration = int64_t(val * 600000000);
            else if(*pUnit == 'h' || *pUnit == 'H')
                duration = int64_t(val * 36000000000);
            else
                duration = (int64_t)val;
        }
        else
            return S_FALSE;
    }
    else
        return S_FALSE;
    return S_OK;
}

void CGraphBuilder::Remove(IFilter* pInput)
{
    uint32_t count = pInput->GetOutputPinCount();
    for(uint32_t i = 0 ; i < count ; ++i)
    {
        dom_ptr<IInputPin> spPinIn;
        dom_ptr<IOutputPin> spPinOut;
        spPinOut = pInput->GetOutputPin(i);
        if(NULL != (spPinIn = spPinOut->GetConnection()))
        {
            do
            {
                Remove(spPinIn->GetFilter());
            }while(NULL != (spPinIn = spPinIn->Next()));
        }
        spPinOut->Disconnect();
    }
    m_spFG->Remove(pInput);
}

void* CGraphBuilder::Process(void* pParam)
{
    CGraphBuilder* pThis = (CGraphBuilder*)pParam;
    pThis->Process();
    return NULL;
}

void CGraphBuilder::Process()
{
//    printf("%s%sinput:%d/%d progress:%.2f%% speed:%.2fX length:%ldms buffer:%ldms\n",
//        m_status.isLive ? "live " : "",
//        m_status.isCycle ? "cycle " : "",
//        m_status.number,m_status.count,
//        m_status.progress,m_status.speed,
//        m_status.time/10000,
//        m_status.buffer/10000);
    sleep(1);
}

bool CGraphBuilder::GetOutputUrl(string& format,int64_t timestamp,uint32_t index)
{
    string result;
    size_t pos = 0,end;
    time_t long_time = time(NULL);
    const tm* pTM = localtime(&long_time);

    while(string::npos != (end = format.find_first_of("%%(",pos)))
    {
        result += format.substr(pos,end-pos);
        pos = end + 2;
        if(string::npos == (end = format.find_first_of(')',pos)))
            return false;
        string name = format.substr(pos,end-pos);
        pos = end + 1;
        if(name == "date")
        {
            char date[11];
            snprintf(date,11,"%04d-%02d-%02d",pTM->tm_year+1900,pTM->tm_mon+1,pTM->tm_mday);
            result += date;
        }
        else if(name == "time")
        {
            char time[9];
            snprintf(time,9,"%02d-%02d-%02d",pTM->tm_hour,pTM->tm_min,pTM->tm_sec);
            result += time;
        }
        else if(name == "timestamp")
        {
            char time[20];
            snprintf(time,20,"%ld",timestamp);
            result += time;
        }
        else if(name == "index")
        {
            char str[20];
            snprintf(str,20,"%d",index);
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

HRESULT CGraphBuilder::GetInputInfo(Object::Ptr obj,IFilter* pFilter,HRESULT hr)
{
    if(IS_OK(hr))
    {
        dom_ptr<IProfile> spProfile;
        if(NULL != (spProfile.p = static_cast<IProfile*>(pFilter->Query(IID(IProfile)))))
        {
            GetInfo(obj,spProfile);
        }
        uint32_t count = pFilter->GetOutputPinCount();
        Array::Ptr streams(new Array());
        obj->set("streams",streams);
        for(uint32_t index = 0 ; index < count ; ++index)
        {
            dom_ptr<IOutputPin> spPin = pFilter->GetOutputPin(index);
            if(spPin != NULL)
            {
                dom_ptr<IMediaType> spMT;
                dom_ptr<IProfile> spInfo;
                JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
                JIF(spPin->GetMediaType(spMT));
                Object::Ptr stream(new Object());
                Object::Ptr info(new Object());
                stream->set(spMT->GetMajorName(),info);
                info->set("id",spMT->GetSubName());
                if(true == spMT.Query(&spInfo))
                    GetInfo(info,spInfo);
                streams->add(stream);
            }
        }
    }
    else
    {
        obj->set("result",hr);
        obj->set("descr","load url fail");
    }
    return hr;
}

void CGraphBuilder::GetInfo(Object::Ptr obj,IProfile* pProfile)
{
    IProfile::val* pVal;
    IProfile::Name name = NULL;
    while(NULL != (name = pProfile->Next(name)))
    {
        if(NULL != (pVal = pProfile->Read(name)) && NULL != (pVal->value))
        {
            if(true == STR_CMP(pVal->type,typeid(int).name()))
                obj->set(name,*(int*)pVal->value);
            else if(true == STR_CMP(pVal->type,typeid(int64_t).name()))
                obj->set(name,*(int64_t*)pVal->value);
            else if(true == STR_CMP(pVal->type,typeid(float).name()))
                obj->set(name,*(float*)pVal->value);
            else if(true == STR_CMP(pVal->type,typeid(double).name()))
                obj->set(name,*(double*)pVal->value);
            else if(true == STR_CMP(pVal->type,typeid(char*).name())||
                true == STR_CMP(pVal->type,typeid(const char*).name()))
                obj->set(name,(const char*)pVal->value);
        }
    }
}
HRESULT CGraphBuilder::dump_callback(HRESULT hr,const char* pModule,const char* pContent,void* pTag)
{
    CGraphBuilder* pThis = (CGraphBuilder*)pTag;
    if(NULL == pThis)
        return hr;
    if(NULL == pThis->m_callback)
        return hr;
    if(S_OK > hr)
    {
        Object msg;
        msg.set("msg","error");
        Object error;
        error.set("return",hr);
        error.set("module",pModule);
        error.set("content",NULL == pContent ? "empty" : pContent);
        msg.set("error",error);
        Dynamic::Var var = msg;
        pThis->m_msg_error = var.toString();
        pThis->m_callback->OnMsg(pThis->m_msg_error.c_str());
    }
    else if(S_OK < hr)
    {
        Object msg;
        msg.set("msg","warring");
        Object warring;
        warring.set("level",hr);
        warring.set("module",pModule);
        warring.set("content",NULL == pContent ? "empty" : pContent);
        msg.set("warring",warring);
        Dynamic::Var var = msg;
        string str = var.toString();
        pThis->m_callback->OnMsg(str.c_str());
    }
    return hr;
}

