#include "GosunSession.h"
#include <Url.cpp>
CStream::CStream(CGosunSession* pSession)
:m_session(pSession)
,m_is_live(false)
,m_index_recv(0)
,m_index_send(0)
,m_index_slice(0)
,m_pos_send(0)
,m_start_recv(MEDIA_FRAME_NONE_TIMESTAMP)
,m_start_send(MEDIA_FRAME_NONE_TIMESTAMP)
,m_day_save(0)
,m_flag(0)
{

}

CStream::~CStream()
{
    Close();
}

bool CStream::Create(void* pParam)
{
    m_is_live = true;
    return m_spStream.Create(CLSID_CNetworkStream,(ICallback*)m_session,false,pParam);
}

HRESULT CStream::Load(const char* pUrl,uint32_t mode)
{
    JCHK(NULL != pUrl,E_INVALIDARG);

    HRESULT hr;
    m_index_recv = 0;
    m_index_send = 0;
    m_index_slice = 0;
    m_pos_send = 0;
    m_start_recv = MEDIA_FRAME_NONE_TIMESTAMP;
    m_start_send = MEDIA_FRAME_NONE_TIMESTAMP;
    m_time_send  = MEDIA_FRAME_NONE_TIMESTAMP;
    m_mode = mode;
    m_root = GetRoot();
    if('/' != m_root.at(m_root.size()-1))
        m_root += '/';

    JCHK(m_spStream == NULL,E_FAIL);
    JIF(m_url.Set(pUrl));
    if(true == (m_is_live = (false == m_url.m_protocol.empty())))
    {
        JCHK(m_spStream.Create(CLSID_CNetworkStream,(ICallback*)m_session),E_FAIL);
        m_name = m_url.GetStreamID(NULL,false);
        m_url.m_port = GOSUN_PORT;
        m_flag = IFilter::FLAG_LIVE;
    }
    else
    {
        JCHK(m_spStream.Create(CLSID_CFileStream,(ICallback*)m_session),E_FAIL);
        JCHK(m_spSlice.Create(CLSID_CFileStream,(ICallback*)m_session),E_FAIL);
        m_name = m_url.Get(false);
        m_flag = 0;
    }
    return hr;
}

HRESULT CStream::SetName(const char* pName)
{
    HRESULT hr;
    JCHK(NULL != pName,E_INVALIDARG);
    JIF(m_url.SetStreamID(GOSUN_PROTOCOL_NAME,pName));
    m_name = pName;
    return hr;
}

const char* CStream::GetName()
{
    return m_name.c_str();
}

uint32_t CStream::GetFlag()
{
    return m_flag;
}

HRESULT CStream::Open()
{
    HRESULT hr = S_OK;
    if(false == m_is_live)
    {
        if(FT_Source == m_mode && false == m_clip.empty())
        {
            JCHK(m_spFrameRead != NULL,E_FAIL);

            IProfile* pSlices = NULL;
            int slice_count = 0;

            dom_ptr<IProfile> spProfile;
            JCHK(m_session->m_pinOut != NULL,E_FAIL);
            JCHK(spProfile.QueryFrom(m_session->m_pinOut->GetMediaType()),E_FAIL);

            if(0 >= m_url.m_duration)
            {
                spProfile->Read("buffer", slice_count);
                if(0 == slice_count)
                    slice_count = 3;
            }

            IProfile::val* pVal;
            JCHK(NULL != (pVal = spProfile->Write("slices",IID(IProfile),NULL,0)),E_FAIL);
            JCHK(pSlices = (IProfile*)pVal->value,E_FAIL);

            msg* pMsg;
            char key[30];
            string path;
            JCHK(NULL != (pMsg = (msg*)m_spFrameRead->info.pExt),E_FAIL);

            snprintf(key,30,"%010u",pMsg->index);
            path = GetSlice(m_clip,pMsg->time);
            JCHK(pSlices->Write(key,path.c_str(),path.size()+1),E_FAIL);
            printf("insert key:%s path:%s\n",key,path.c_str());
            if(0 < slice_count)
            {
                while(pSlices->Count() <= (uint32_t)slice_count)
                {
                    dom_ptr<IMediaFrame> spFrame;
                    JIF(Read(m_spStream,&spFrame));
                    JCHK(NULL != (pMsg = (msg*)spFrame->info.pExt),E_FAIL);

                    if(mt_slice == pMsg->type)
                    {
                        snprintf(key,30,"%010u",pMsg->index);
                        path = GetSlice(m_clip,pMsg->time);
                        JCHK(pSlices->Write(key,path.c_str(),path.size()+1),E_FAIL);
                        printf("insert key:%s path:%s\n",key,path.c_str());
                        if(PACKET_END_SLICE_INDEX == pMsg->index)
                            break;
                    }
                }
            }
            else if(0 < m_url.m_duration)
            {
                while(true)
                {
                    dom_ptr<IMediaFrame> spFrame;
                    JIF(Read(m_spStream,&spFrame));
                    JCHK(NULL != (pMsg = (msg*)spFrame->info.pExt),E_FAIL);

                    if(mt_slice == pMsg->type)
                    {
                        if(pMsg->time > m_url.m_pos + m_url.m_duration)
                        {
                            break;
                        }
                        else
                        {
                            snprintf(key,30,"%010u",pMsg->index);
                            path = GetSlice(m_clip,pMsg->time);
                            JCHK(pSlices->Write(key,path.c_str(),path.size()+1),E_FAIL);
                            printf("insert key:%s path:%s\n",key,path.c_str());
                        }

                        if(PACKET_END_SLICE_INDEX == pMsg->index)
                            break;
                    }
                }
                path.clear();
                JCHK(pSlices->Write("",path.c_str(),path.size()+1),E_FAIL);
            }
        }
    }
    else
    {
        if(FT_Render == m_mode)
        {
            JIF(m_spStream->Open(m_url.Get(),FT_Render));
        }
    }
    return hr;
}

HRESULT CStream::Play()
{
    if(FT_Source == m_mode)
    {
        if(true == m_is_live)
            return m_spStream->SetEventEnable(true);
        else
            return m_spSlice->SetEventEnable(true);
    }
    else if(FT_Render == m_mode)
    {
        if(true == m_is_live)
            return S_FALSE;
        else
            return S_OK;
    }
    else if(true == m_is_live)
        return m_spStream->SetEventEnable(true);
    return E_FAIL;
}

HRESULT CStream::Pause()
{
    if(FT_Source == m_mode)
    {
        if(true == m_is_live)
            return m_spStream->SetEventEnable(false);
        else
            return m_spSlice->SetEventEnable(false);
    }
    else if(FT_Render == m_mode)
    {
        if(true == m_is_live)
            return S_FALSE;
        else
            return S_OK;
    }
    else if(true == m_is_live)
        return m_spStream->SetEventEnable(false);
    return E_FAIL;
}

void CStream::Close()
{
    if(m_spSlice != NULL)
    {
        m_spSlice->Close();
        m_spSlice = NULL;
    }
    if(m_spStream != NULL)
    {
        m_spStream->Close();
        m_spStream = NULL;
    }
}

HRESULT CStream::Read(IMediaFrame** ppFrame)
{
    if(true == m_is_live)
        return Read(m_spStream,ppFrame);
    else
    {
        HRESULT hr;
        msg* pMsg;
        dom_ptr<IMediaFrame> spFrame;
        JCHK(m_spSlice != NULL,E_FAIL);
        JIF(Read(m_spSlice,&spFrame));
        JCHK(pMsg = (msg*)spFrame->info.pExt,E_FAIL);
        if(mt_tail == pMsg->type)
        {
            if(PACKET_END_SLICE_INDEX == pMsg->index || true == m_clip.empty())
            {
                spFrame->info.flag |= MEDIA_FRAME_FLAG_EOF;
                spFrame->Clear();
                JIF(m_session->m_pinOut->Write(spFrame));
                return E_AGAIN;
            }
            else
            {
                string path = GetSlice(m_slice,pMsg->time);
                JIF(m_spSlice->Open(path.c_str(),m_mode));
                return Read(m_spSlice,ppFrame);
            }
        }
        else
        {
            return spFrame.CopyTo(ppFrame);
        }
    }
}

HRESULT CStream::Write(IMediaFrame* pFrame)
{
    HRESULT hr;
    msg* pPacket;
    JCHK(NULL != pFrame,E_INVALIDARG);
    JCHK(pPacket = (msg*)pFrame->info.pExt,E_INVALIDARG);

    if(MEDIA_FRAME_NONE_TIMESTAMP == pFrame->info.dts)
        pFrame->info.dts = GetTickCount();

    pPacket->time = pFrame->info.dts /10000;

    if(true == m_is_live)
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP == m_start_send)
        {
            m_index_send = pPacket->index;
            m_start_send = pFrame->info.dts;
        }
        else
        {
            JCHK(pPacket->index == m_index_send,E_FAIL);
        }
        JIF(m_spStream->Write(pFrame,0,IStream::WRITE_FLAG_FRAME));
        ++m_index_send;
        if(0 != (pPacket->flag&MEDIA_FRAME_FLAG_SEGMENT))
        {
//            LOG(0,"send packet index:%d time:%ldms type:%d flag:%d payload size:%d",
//                pPacket->index,pPacket->time,pPacket->type,
//                pPacket->flag,pPacket->size);
        }
    }
    else
    {
        if(mt_push == pPacket->type)
        {
            return m_session->Send(mt_info,NULL);
        }
        else if(mt_info == pPacket->type)
        {
            m_spMediaInfo = pFrame;

            if(false == m_spStream->IsOpen())
            {
                m_day_save = 0;
                dom_ptr<IProfile> spProfile;
                JCHK(spProfile.QueryFrom(m_session->m_pinIn->GetMediaType()),E_FAIL);\

                IProfile::val* pVal = spProfile->Read("duration");
                if(NULL != pVal)
                {
                    char* pUnit = NULL;
                    double val = strtod((const char*)pVal->value, &pUnit);
                    if(0.0 < val)
                    {
                        if(*pUnit == 'd' || *pUnit == 'D')
                            m_day_save = (uint32_t)val;
                    }
                }
            }
            else
            {
                JIF(m_spStream->Write(pFrame,0,IStream::WRITE_FLAG_FRAME));
            }
        }
        else if(mt_packet == pPacket->type)
        {
            bool is_segment = 0 != (pFrame->info.flag&(MEDIA_FRAME_FLAG_NEWSEGMENT|MEDIA_FRAME_FLAG_SEGMENT));
            if(is_segment)
            {
                //printf("CStream::Write dts:%ld flag:%u\n",pFrame->info.dts,pFrame->info.flag);
                msg slice;
                slice = *pPacket;
                slice.size = 0;
                string path;

                if(true == m_spSlice->IsOpen())
                {
                    slice.type = mt_tail;
                    JIF(m_spSlice->Write(&slice,sizeof(slice)));
                }
                else
                {
                    JCHK(m_spMediaInfo != NULL,E_FAIL);

                    char file[30];
                    snprintf(file,30,"%ld",pPacket->time);
                    m_slice = m_name;
                    m_slice += '/';
                    m_slice += file;
                    path = m_slice;
                    path += INDEX_SECTION_NAME;
                    JIF(m_spStream->Open(path.c_str(),m_mode));
                    m_slice += SLICE_SECTION_NAME;
                    m_slice += '/';
                }
                slice.type = mt_slice;
                slice.index = m_index_slice++;
                JIF(m_spStream->Write(&slice,sizeof(slice)));

                path = GetSlice(m_slice,pPacket->time);
                JIF(m_spSlice->Open(path.c_str(),m_mode));
                JIF(m_spSlice->Write(m_spMediaInfo.p,0,IStream::WRITE_FLAG_FRAME));
                m_pos_send = 0;

                if(MEDIA_FRAME_NONE_TIMESTAMP == m_start_send)
                    m_start_send = pFrame->info.dts;
            }
            else
            {
                JCHK(true == m_spSlice->IsOpen(),E_FAIL);
            }

            pPacket->index = m_pos_send;
            uint32_t pos = (uint32_t)m_spSlice->GetPos();
            JIF(m_spSlice->Write(pFrame,0,IStream::WRITE_FLAG_FRAME));

            if(0 != (pFrame->info.flag&MEDIA_FRAME_FLAG_SYNCPOINT))
                m_pos_send = pos;
            m_time_send = pFrame->info.dts + pFrame->info.duration;
        }
        else if(mt_pull == pPacket->type)
        {
            if(FT_Source == m_mode)
            {
                JIF(Seek());
            }
        }
        else if(mt_play == pPacket->type)
        {
            JIF(m_spSlice->SetEventEnable(true));
        }
        else if(mt_pause == pPacket->type)
        {
            JIF(m_spSlice->SetEventEnable(false));
        }
    }
    return hr;
}

HRESULT CStream::CanWrite()
{
    if(NULL == m_spStream)
        return E_EOF;
    else if(false == m_spStream->IsOpen())
        return S_OK;
    else
        return true == m_spStream->CanWrite() ? S_OK : E_AGAIN;
}

void CStream::DirDelete(string path)
{
    if(path.empty())
        return;
    if('/' != path.at(path.size()-1))
        path += '/';

    DIR *dir;
    if(NULL != (dir = opendir(path.c_str())))
    {
        dirent *cur;
        while(NULL != (cur = readdir(dir)))
        {
            string file = path;
            file += cur->d_name;
            if(4 == cur->d_type)
                DirDelete(file);
            else
                unlink(file.c_str());
        }
        closedir(dir);
    }
    rmdir(path.c_str());
}

void CStream::ClipDelete(tm* time)
{
//    string path = m_root;
//    path += m_clip;
//
//    string target;
//
//    if(m_clips.empty())
//    {
//        DIR *dir;
//        if(NULL != (dir = opendir(path.c_str())))
//        {
//            dirent *cur;
//            while(NULL != (cur = readdir(dir)))
//            {
//                if(DT_DIR == cur->d_type)
//                {
//                    char* pEnd = NULL;
//                    int64_t time = strtoll(cur->d_name,&pEnd,10);
//                    if(0 == strcmp(CLIP_SECTION_NAME,pEnd))
//                    {
//                        target = path;
//                        target += cur->d_name;
//                        target.erase(target.size() - strlen(pEnd));
//                        m_clips.insert(ClipPair(time,target));
//                    }
//                }
//            }
//            closedir(dir);
//        }
//    }
}

HRESULT CStream::Read(IStream* pStream,IMediaFrame** ppFrame)
{
    JCHK(NULL != ppFrame,E_INVALIDARG);
    JCHK(m_session->m_spAllocate != NULL,E_FAIL);

    HRESULT hr;
    msg* pMsg;
    const IMediaFrame::buf* pBuf;
    if(m_spFrameRecv == NULL)
    {
        dom_ptr<IMediaFrame> spFrame;

        JIF(m_session->m_spAllocate->Alloc(&spFrame));
        JIF(spFrame->SetBuf(0,sizeof(msg)));
        JCHK(pBuf = spFrame->GetBuf(),E_FAIL);
        JIF(pStream->Read(pBuf->data,pBuf->size));
        pMsg = (msg*)pBuf->data;


        if(MEDIA_FRAME_NONE_TIMESTAMP == m_start_recv)
        {
            m_index_recv = pMsg->index;
            m_start_recv = pMsg->time;
        }

        if(true == m_is_live)
        {
            JCHK(pMsg->index == m_index_recv++,E_FAIL);
        }
        else if(mt_packet == pMsg->type)
        {
            pMsg->index = m_index_recv++;
        }

        if(0 < pMsg->size)
        {
            JIF(spFrame->SetBuf(1,pMsg->size));
        }

        spFrame->info.msg = pMsg->type;
        spFrame->info.flag = pMsg->flag;
        spFrame->info.dts = pMsg->time*10000;
        spFrame->info.pts = spFrame->info.dts;
        spFrame->info.tag = pMsg->index;
        spFrame->info.pExt = pMsg;
        m_spFrameRecv = spFrame;
    }

    if(NULL != (pBuf = m_spFrameRecv->GetBuf(1)))
    {
        JIF(pStream->Read(pBuf->data,pBuf->size));
    }
    if(0 != (pMsg->flag&MEDIA_FRAME_FLAG_SEGMENT))
    {
//        LOG(0,"recv packet index:%d time:%ldms dts:%ld type:%d flag:%d payload size:%d",
//                pMsg->index,pMsg->time,m_spFrameRecv->info.dts,pMsg->type,pMsg->flag,pMsg->size);
    }
    JIF(m_spFrameRecv.CopyTo(ppFrame));
    m_spFrameRecv = NULL;
    return hr;
}

string CStream::GetSlice(const string& root,int64_t pos)
{
    time_t second = pos / 1000;
    tm* now = localtime(&second);
    char name[60];
    snprintf(name,60,SLICE_NAME_FORMAT,1900 + now->tm_year,now->tm_mon,now->tm_mday,pos);
    string path = root;
    path += name;
    return path;
}

HRESULT CStream::Seek()
{
    HRESULT hr;
    string path = m_root;
    path += m_name;
    struct stat buf;
    if(0 != stat(path.c_str(),&buf))
        return E_FAIL;
    if(S_ISREG(buf.st_mode))
    {
        size_t pos = path.rfind(SLICE_SECTION_NAME);
        JCHK(pos != string::npos,E_FAIL);
        m_slice = path.substr(0,pos+strlen(SLICE_SECTION_NAME)+1);
        path = m_slice;
        path.replace(pos,string::npos,INDEX_SECTION_NAME);
        m_url.m_pos = strtoll(m_url.m_file.c_str(),NULL,10);
        m_clip.clear();
    }
    else if(S_ISDIR(buf.st_mode))
    {
        path += '/';

        DIR *dir;
        if(NULL == (dir = opendir(path.c_str())))
            return E_FAIL;

        string name;
        int64_t pos = MEDIA_FRAME_NONE_TIMESTAMP;
        dirent *cur;
        while(NULL != (cur = readdir(dir)))
        {
            if(DT_UNKNOWN == cur->d_type || DT_REG == cur->d_type)
            {
                char* pEnd = NULL;
                int64_t time = strtoll(cur->d_name,&pEnd,10);
                if(pEnd > cur->d_name && 0 == strcmp(INDEX_SECTION_NAME,pEnd))
                {
                    if(0 <= m_url.m_pos)
                    {
                        if(time <= m_url.m_pos)
                        {
                            if(time == m_url.m_pos)
                            {
                                pos = time;
                                name = cur->d_name;
                                break;
                            }
                            else if(MEDIA_FRAME_NONE_TIMESTAMP == pos || time > pos)
                            {
                                pos = time;
                                name = cur->d_name;
                            }
                        }
                    }
                    else
                    {
                        if(MEDIA_FRAME_NONE_TIMESTAMP == pos || time < pos)
                        {
                            pos = time;
                            name = cur->d_name;
                        }
                    }
                }
            }
        }
        closedir(dir);

        if(MEDIA_FRAME_NONE_TIMESTAMP == pos)
            return E_FAIL;

        m_slice = path;
        path += name;
        name.replace(name.size()-strlen(INDEX_SECTION_NAME),string::npos,SLICE_SECTION_NAME);
        m_slice += name;
        m_slice += '/';

        m_clip += m_url.m_file;
        if(false == m_url.m_format.empty())
        {
            m_clip += '.';
            m_clip += m_url.m_format;
        }
        m_clip += '/';
        m_clip += name;
        m_clip += '/';
    }
    else
        return E_FAIL;

    JIF(m_spStream->Open(path.c_str(),FT_Source));

    msg* pMsgFirst;
    dom_ptr<IMediaFrame> spFirst;
    JIF(Read(m_spStream,&spFirst));
    JCHK(NULL != (pMsgFirst = (msg*)spFirst->info.pExt),E_FAIL);
    JCHK(mt_slice == pMsgFirst->type,E_FAIL);
    JCHK(m_url.m_pos >= pMsgFirst->time,E_FAIL);

    int64_t sz = m_spStream->GetPos();

    if(m_url.m_pos == spFirst->info.dts)
    {
        m_spFrameRead = spFirst;
    }
    else
    {
        msg* pMsgLast;
        dom_ptr<IMediaFrame> spLast;
        JIF(m_spStream->Seek(-sz,IStream::end));
        JIF(Read(m_spStream,&spLast));
        JCHK(NULL != (pMsgLast = (msg*)spLast->info.pExt),E_FAIL);
        JCHK(mt_slice == pMsgLast->type,E_FAIL);

        JCHK(pMsgLast->time > pMsgFirst->time,E_FAIL);
        JCHK(pMsgLast->index > pMsgFirst->index,E_FAIL);

        int64_t duration = int64_t(double(pMsgLast->time - pMsgFirst->time)/(pMsgLast->index - pMsgFirst->index) + 0.5);
        if(m_url.m_pos >= pMsgLast->time && m_url.m_pos < pMsgLast->time + duration)
        {
            m_spFrameRead = spLast;
        }
        else
        {
            int64_t pos = (m_url.m_pos - pMsgFirst->time) / duration * sz;
            JIF(m_spStream->Seek(pos));

            while(true)
            {
                msg* pMsg;
                dom_ptr<IMediaFrame> spFrame;
                JIF(Read(m_spStream,&spFrame));
                JCHK(NULL != (pMsg = (msg*)spFrame->info.pExt),E_FAIL);
                JCHK(mt_slice == pMsg->type,E_FAIL);
                if(pMsg->time <= m_url.m_pos)
                {
                    m_spFrameRead = spFrame;
                    if(pMsg->time == m_url.m_pos)
                        break;
                }
                else if(m_spFrameRead != NULL)
                {
                    JIF(m_spStream->Seek(-sz,IStream::current));
                    break;
                }
                else
                {
                    JIF(m_spStream->Seek(-(sz*2),IStream::current));
                }
            }
        }
    }
    JCHK(NULL != (pMsgFirst = (msg*)m_spFrameRead->info.pExt),E_FAIL);
    path = GetSlice(m_slice,pMsgFirst->time);
    JIF(m_spSlice->Open(path.c_str(),m_mode));
    JIF(m_spSlice->SetEventEnable(true));
    return hr;
}

string CStream::GetRoot()
{
    string root = "/";
    IProfile::val* pVal;
    if(NULL != (pVal = g_pSite->GetProfile()->Read("root")))
    {
        if(true == STR_CMP(pVal->type,typeid(char*).name()) || true == STR_CMP(pVal->type,typeid(const char*).name()))
            root = (char*)pVal->value;
    }
    return root;
}

CGosunSession::CGosunSession()
:m_type(FT_None)
,m_status(S_Stop)
,m_pTag(NULL)
,m_stream(this)
,m_index_send(0)
,m_timeout(0)
{
    memset(&m_packet,0,sizeof(m_packet));
//    memset(&m_stream_status,0,sizeof(m_stream_status));
}

bool CGosunSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    if(NULL != pParam)
    {
        JCHK(m_stream.Create(pParam),false);
    }
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    JCHK(m_spAllocate.Create(CLSID_CMediaFrameAllocate),false);
    return true;
}

bool CGosunSession::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Notify(S_Stop);
        LOG(0,"%s[%p] name:%s release",Class().name,(IFilter*)this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CGosunSession)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CGosunSession::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    HRESULT hr;
    if(NULL == pUrl)
    {
        pUrl = m_name.c_str();
    }
    JIF(m_stream.Load(pUrl,mode));
    m_name = m_stream.GetName();
    JIF(SetType((FilterType)mode));
    if(FT_Source == mode)
    {
        JIF(Send(mt_pull));
    }
    return hr;
}

STDMETHODIMP_(FilterType) CGosunSession::GetType()
{
    return m_type;
}

STDMETHODIMP CGosunSession::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CGosunSession::GetName()
{
    return true == m_name.empty() ? NULL : m_name.c_str();
}

STDMETHODIMP_(uint32_t) CGosunSession::GetFlag()
{
    return m_stream.GetFlag();
}

STDMETHODIMP_(uint32_t) CGosunSession::GetInputPinCount()
{
    return m_pinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CGosunSession::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn.p : NULL;
}

STDMETHODIMP_(uint32_t) CGosunSession::GetOutputPinCount()
{
    return m_pinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CGosunSession::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut.p : NULL;
}

STDMETHODIMP_(IInputPin*) CGosunSession::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CGosunSession::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CGosunSession::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(FT_Render == m_type)
            {
                if(S_Stop == m_status)
                {
                    JIF(m_stream.Open());
                    if(mt_none == m_packet.type)
                    {
                        JIF(Send(mt_push));
                    }
                    else if(mt_pull == m_packet.type)
                    {
                        IMediaFrame* pFrame;
                        JCHK(pFrame = m_pinIn->GetMediaType()->GetFrame(),E_FAIL);
                        JIF(Send(mt_info,pFrame));
                    }
                }
                else if(S_Stop == cmd)
                {
                    m_stream.Close();
                }
                else if(S_Play == cmd)
                {
                    JIF(m_stream.Play());
                    if(S_OK != hr)
                        cmd = S_Pause;
                }
            }
            else if(FT_Source == m_type)
            {
                if(S_Stop == m_status)
                {
                    JIF(m_stream.Open());
                    if(mt_push == m_packet.type)
                    {
                        JIF(Send(mt_pull));
                    }
                }
                else if(S_Stop == cmd)
                {
                    m_stream.Close();
                }
                else
                {
                    JIF(Send(S_Play == cmd ? mt_play : mt_pause));
                }
             }
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    else
    {
        if(IFilter::C_Profile == cmd)
        {
            JIF(Init());
        }
        else if(IFilter::C_Accept == cmd)
        {
            JIF(m_stream.Play());
        }
        else if(IFilter::C_Enable == cmd)
        {
        }
        else if(IFilter::C_Disable == cmd)
        {
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CGosunSession::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CGosunSession::SetTag(void* tag)
{
    m_pTag = tag;
}

STDMETHODIMP_(void*) CGosunSession::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CGosunSession::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CGosunSession::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    return NULL == pMT || COMPARE_SAME_VALUE == pPin->GetMediaType()->Compare(pMT) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CGosunSession::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return NULL == pMT || COMPARE_SAME_VALUE == pPin->GetMediaType()->Compare(pMT) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CGosunSession::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return Send(mt_packet,pFrame);
}

//ICallback
STDMETHODIMP CGosunSession::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(ET_EOF == type)
    {
        //if(0 < m_timeout)
        //    return m_stream.SetTimer(0,m_timeout,true);
    }
    else if(ET_Stream_Read == type)
        return Recv();
    else if(ET_Stream_Write == type)
    {
        if(FT_Render == m_type)
        {
            if(NULL == m_pinIn)
                return E_AGAIN;

            HRESULT hr;
            JIF(m_pinIn->Write(NULL));
            param2 = m_pinIn.p;
        }
        else if(FT_Source == m_type)
        {
            HRESULT hr;
            JIF(m_stream.Play());
            return E_AGAIN;
        }
    }
    else if(ET_Epoll_Timer == type)
    {
        //uint64_t start_ms = *(uint64_t*)param2;
        //uint64_t clock_ms = *(uint64_t*)param3;
        if(0 == param1)
        {
            m_ep->Notify(ET_EOF,E_EOF,param2,param3);
            return E_EOF;
        }
        else
            return S_OK;
    }
    else if(ET_Filter_Render == type)
    {
        if(0 == param1)
        {
            HRESULT hr;
            JIF(m_stream.Pause());
        }
        return param1;
    }
    return m_ep->Notify(type,param1,param2,param3);
}

HRESULT CGosunSession::SetType(FilterType type)
{
    JCHK(FT_Source == type || FT_Render == type,E_INVALIDARG);
    JCHK(FT_None == m_type,E_FAIL);

    HRESULT hr = S_OK;
    if(type == m_type)
        return hr;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
    JIF(spMT->SetSub(MST_GOSUN_DATA));

    if(FT_Source == type)
    {
        JCHK(m_pinOut.Create(CLSID_COutputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinOut->SetMediaType(spMT));
    }
    else if(FT_Render == type)
    {
        JCHK(m_pinIn.Create(CLSID_CInputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinIn->SetMediaType(spMT));
    }

    m_type = type;
    return hr;
}

HRESULT CGosunSession::Recv()
{
    HRESULT hr;
    dom_ptr<IMediaFrame> spFrame;

    JIF(m_stream.Read(&spFrame));

    dom_ptr<IStream> spStream;
    JCHK(spFrame.Query(&spStream),E_FAIL);

    JIF(spStream->Read(&m_packet,sizeof(m_packet)));

    switch(m_packet.type)
    {
        case mt_push:
        {
            OBJ_VERSION ver;
            JIF(spStream->Read(&ver,sizeof(ver)));
            JCHK(ver == Class().version,E_FAIL);

            uint32_t len;
            JIF(spStream->Read(&len,sizeof(len)));
            JCHK(0 < len,E_FAIL);

            m_name.resize(len,0);
            JIF(spStream->Read((void*)m_name.data(),len));
            JIF(m_stream.SetName(m_name.c_str()));
            JIF(SetType(FT_Source));
            JIF(m_ep->Notify(ET_Session_Push,0,m_pinOut.p));
        }
        break;
        case mt_pull:
        {
            if(FT_Render != m_type)
            {
                OBJ_VERSION ver;
                JIF(spStream->Read(&ver,sizeof(ver)));
                JCHK(ver == Class().version,E_FAIL);

                uint32_t len;
                JIF(spStream->Read(&len,sizeof(len)));
                JCHK(0 < len,E_FAIL);

                m_name.resize(len,0);
                JIF(spStream->Read((void*)m_name.data(),len));
                JIF(m_stream.SetName(m_name.c_str()));

                JIF(SetType(FT_Render));

                JIF(m_ep->Notify(ET_Session_Pull,0,m_pinIn.p));
            }
            else
            {
                JIF(Send(mt_info,m_pinIn->GetMediaType()->GetFrame()));
            }
        }
        break;
        case mt_info:
        {
            if(FT_Source != m_type)
            {
                JIF(SetType(FT_Source));
            }

            JCHK(m_pinOut != NULL,E_FAIL);

            dom_ptr<IMediaType> spMT;
            JCHK(spMT = m_pinOut->GetMediaType(),E_FAIL);
            if(NULL == spMT->GetFrame())
            {
                JIF(spMT->SetFrame(spFrame));
                JIF(m_ep->Notify(ET_Filter_Build));
            }
            else
            {
                JIF(spMT->SetFrame(spFrame));
            }
        }
        break;
        case mt_play:
        {
            JCHK(FT_Render == m_type,E_FAIL);
            JCHK(m_pinIn != NULL,E_FAIL);

            m_status = S_Play;
            JIF(m_pinIn->Send(S_Play,false,true));
            m_pinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
            JIF(m_ep->Notify(ET_Filter_Buffer,0,m_pinIn.p));
        }
        break;
        case mt_pause:
        {
            JCHK(FT_Render == m_type,E_FAIL);
            JCHK(m_pinIn != NULL,E_FAIL);
            if(S_Pause != m_status)
            {
                m_status = S_Pause;
                JIF(m_pinIn->Send(S_Pause,false,false));
            }
        }
        break;
        case mt_packet:
        {
            JCHK(FT_Source == m_type,E_FAIL);
            JIF(m_pinOut->Write(spFrame));
        }
        break;
        case mt_fail:
        {

        }
        break;
        case mt_tail:
        {
            return E_EOF;
        }
        break;
        default:
        {
            JCHK0(false,E_FAIL,"unknown packet");
        }
    }
    return hr;
}

HRESULT CGosunSession::Send(msg_type send,IMediaFrame* pFrame)
{
    HRESULT hr;
    JIF(m_stream.CanWrite());

    msg* pMsg;

    dom_ptr<IMediaFrame> spFrame;

    if(NULL != pFrame && NULL != pFrame->info.pExt)
    {
        pMsg = (msg*)pFrame->info.pExt;
        pMsg->index = m_index_send++;
        spFrame = pFrame;
    }
    else
    {
        dom_ptr<IStream> spStream;

        JIF(m_spAllocate->Alloc(&spFrame));
        JIF(spFrame.Query(&spStream));

        JIF(spStream->Write(NULL,sizeof(msg),0,(void**)&pMsg));
        if(pFrame != NULL)
        {
            JIF(spStream->Write(pFrame,0,IStream::WRITE_FLAG_FRAME|IStream::WRITE_FLAG_REFFER));
            spFrame->info = pFrame->info;
        }

        switch(send)
        {
            case mt_push:
            {
                JIF(spStream->Write(&Class().version,sizeof(Class().version,IStream::WRITE_FLAG_REFFER)));
                uint32_t len = m_name.size();
                JIF(spStream->Write(&len,sizeof(len)));
                JIF(spStream->Write(m_name.data(),len,IStream::WRITE_FLAG_REFFER));
            }
            break;
            case mt_pull:
            {
                if(mt_push != m_packet.type)
                {
                    JIF(spStream->Write(&Class().version,sizeof(Class().version,IStream::WRITE_FLAG_REFFER)));
                    uint32_t len = m_name.size();
                    JIF(spStream->Write(&len,sizeof(len)));
                    JIF(spStream->Write(m_name.data(),len,IStream::WRITE_FLAG_REFFER));
                }
            }
            break;
            case mt_info:
            {
                dom_ptr<IMediaType> spMT;
                JCHK(FT_Render == m_type,E_FAIL);
                JCHK(m_pinIn != NULL,E_FAIL);
                JCHK(NULL != m_pinIn->GetConnection(),E_FAIL);
                JCHK(spMT = m_pinIn->GetMediaType(),E_FAIL);

                IMediaFrame* pFrame;
                JCHK(pFrame = spMT->GetFrame(),E_FAIL);
                JIF(spStream->Write(pFrame,0,IStream::WRITE_FLAG_FRAME|IStream::WRITE_FLAG_REFFER));
            }
            break;
            case mt_play:
            {
                JCHK(m_pinOut != NULL,E_FAIL);
            }
            break;
            case mt_pause:
            {
                JCHK(m_pinOut != NULL,E_FAIL);
            }
            break;
            case mt_packet:
            {
                JCHK(NULL != pFrame,E_FAIL);
            }
            break;
            case mt_fail:
            {

            }
            break;
            default:
            {

            }
        }
        pMsg->type = send;
        pMsg->index = m_index_send++;
        pMsg->flag = spFrame->info.flag;
        pMsg->time = spFrame->info.dts;

        uint32_t sz;
        JCHK(spFrame->GetBuf(0,NULL,&sz),E_FAIL);
        sz += sizeof(sz);
        JIF(spStream->Write(&sz,sizeof(sz)));
        pMsg->size = sz - sizeof(msg);

        spFrame->info.pExt = pMsg;
        spFrame->info.msg = send;
        spFrame->info.tag = m_index_send;
    }
    JIF(m_stream.Write(spFrame));
    if(mt_packet == pMsg->type)
    {
        JIF(m_ep->Notify(ET_Filter_Render,hr,m_pinIn.p,spFrame.p));
    }
    return hr;
}

HRESULT CGosunSession::Init()
{
    IProfile::val* pVal;
    bool is_catch = false;

    m_spProfile->Read("catch",is_catch);
    if(NULL != (pVal = m_spProfile->Read("timeout")))
    {
        if(STR_CMP(pVal->type,typeid(char*).name()) || STR_CMP(pVal->type,typeid(const char*).name()))
        {
            char* pUnit = NULL;
            double val = strtod((const char*)pVal->value, &pUnit);
            if(0.0 < val)
            {
                if(*pUnit == 'h' || *pUnit == 'H')
                    m_timeout = int64_t(val * 60 * 60 * 1000);
                else if(*pUnit == 'm' || *pUnit == 'M')
                    m_timeout = int64_t(val * 60 * 1000);
                else if(*pUnit == 's' || *pUnit == 'S')
                    m_timeout = int64_t(val * 1000);
                else
                    m_timeout = int64_t(val);
            }
        }
    }
    return S_OK;
}

HRESULT CGosunSession::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(STR_CMP(protocol,GOSUN_PROTOCOL_NAME) ||
            (NULL == protocol && STR_CMP(format,GOSUN_PROTOCOL_NAME)))
        return 1;
    return E_INVALIDARG;
}

HRESULT CGosunSession::CreateListener(IStreamListen** ppListen,Interface* pObj,uint16_t* pPort)
{
    if(NULL != ppListen)
    {
        dom_ptr<IStreamListen> spListen;
        JCHK(spListen.Create(CLSID_CNetworkListen,NULL,false,pObj),E_FAIL);
        spListen.CopyTo(ppListen);
    }
    if(NULL != pPort)
    {
        *pPort = GOSUN_PORT;
    }
    return 1;
}

