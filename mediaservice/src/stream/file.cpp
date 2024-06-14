#include "file.h"
#include "epoll.h"


bool CreateDirectory(const string& path)
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

CFileStream::CFileStream()
:m_callback(NULL)
,m_fd(INVALID_FD)
,m_fd_event(INVALID_FD)
,m_mode(0)
,m_msg_event(0)
,m_pos(0)
,m_flag(SEEK_FLAG)
,m_locker(NULL)
,m_thread_read(0)
{
    memset(&m_status,0,sizeof(m_status));
}

bool CFileStream::FinalConstruct(Interface* pOuter,void* pParam)
{
    m_callback = (ICallback*)pOuter;

    dom_ptr<IEpoll> spEpoll;
    JCHK(spEpoll.QueryFrom(g_pSite->GetObj()),false);
    JCHK(S_OK == spEpoll->CreatePoint(this,&m_ep),false);
    JCHK(NULL != (m_locker = m_ep->GetLocker()),false);

    return true;
}

bool CFileStream::FinalDestructor(bool finally)
{
    if(true == finally)
        Close();
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CFileStream)
DOM_QUERY_IMPLEMENT(IStream)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(uint32_t) CFileStream::GetFlag()
{
    return m_flag;
}

STDMETHODIMP CFileStream::Open(const char* pUrl,uint32_t mode)
{
    JCHK(NULL != pUrl,E_INVALIDARG);
    JCHK(0 < strlen(pUrl),E_INVALIDARG);
    JCHK(FT_Source == mode || FT_Render == mode,E_FAIL);

    HRESULT hr = S_OK;

    Close();

    m_name = pUrl;
    if(pUrl[0] != '/')
    {
        IProfile::val* pVal;
        if(NULL != (pVal = g_pSite->GetProfile()->Read("root")))
        {
            if(true == STR_CMP(pVal->type,typeid(char*).name()) || true == STR_CMP(pVal->type,typeid(const char*).name()))
            {
                char* pName = (char*)pVal->value;
                if(0 < strlen(pName))
                {
                    m_name = pName;
                    if('/' != m_name.at(m_name.size()-1))
                    {
                        m_name += '/';
                    }
                    m_name += pUrl;
                }
            }
        }
    }

    m_mode = mode;
    m_flag = SEEK_FLAG;

    if(FT_Source == mode)
    {
        mode = O_RDONLY;
        m_flag |= READ_FLAG;
    }
    else if(FT_Render == mode)
    {
        JCHK1(CreateDirectory(m_name.c_str()),E_INVALIDARG,"Create directory:[%s] fail",pUrl);
        mode =  O_WRONLY | O_CREAT;
        m_flag |= WRITE_FLAG;
    }

    mode |= O_NONBLOCK;

    JCHK4(INVALID_FD != (m_fd = open(m_name.c_str(),mode,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)),E_FAIL,
        "open file:%s flag:%d fail,error id:%d message:%s",pUrl,mode,errno,strerror(errno));

    m_pos = 0;
    memset(&m_status,0,sizeof(m_status));
    return hr;
}

STDMETHODIMP_(void) CFileStream::Close()
{
    CLocker locker(m_locker);
    if(INVALID_FD != m_fd)
    {
        SetEventEnable(false);
        close(m_fd);
        m_fd = INVALID_FD;
    }
}

STDMETHODIMP CFileStream::SetEventEnable(bool enable)
{
    HRESULT hr;

    CLocker locker(m_locker);

    if(true == enable)
    {
        if(INVALID_FD == m_fd_event)
        {
            //printf("thread:%ld m_ep->Add(m_fd_event)\n",pthread_self());
            JCHK(INVALID_FD != (m_fd_event = eventfd(0,EFD_NONBLOCK)),E_FAIL);
            JIF(m_ep->Add(m_fd_event));
            write(m_fd_event,&m_msg_event,sizeof(m_msg_event));
        }
    }
    else
    {
        if(INVALID_FD != m_fd_event)
        {
            //printf("thread:%ld m_ep->Del(m_fd_event)\n",pthread_self());
            read(m_fd_event,&m_msg_event,sizeof(m_msg_event));
            JIF(m_ep->Del(m_fd_event));
            close(m_fd_event);
            m_fd_event = INVALID_FD;
        }
    }
    m_thread_read = 0;
    return hr;
}

STDMETHODIMP_(bool) CFileStream::GetEventEnable()
{
    return INVALID_FD != m_fd_event;
}

STDMETHODIMP CFileStream::SetTimer(int id,uint32_t duration,bool one_shoot)
{
    JCHK(m_ep != NULL,E_FAIL);
    return m_ep->SetTimer(id,duration,one_shoot);
}

STDMETHODIMP_(bool) CFileStream::CanRead()
{
    return true;
}

STDMETHODIMP_(bool) CFileStream::CanWrite()
{
    return true;
}

STDMETHODIMP CFileStream::Read(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf)
{
    JCHK(NULL != pBuf,E_INVALIDARG);

    JCHK(0 < szBuf,E_INVALIDARG);

    if(m_locker == NULL)
        return E_EOF;

    CLocker locker(m_locker);

    ssize_t sz = read(m_fd,pBuf,szBuf);

    if(0 > sz)
    {
        LOG(0,"%s[%p]:[%s] read fail,error id:%d,message:%s",Class().name,this,m_name.c_str(),errno,strerror(errno));
        return E_FAIL;
    }
    else if(0 == sz)
    {
        LOG(0,"%s[%p]:[%s] read end",Class().name,this,m_name.c_str());
        return E_EOF;
    }
    else
    {
        m_pos += sz;
        m_status.read_total_size += sz;
    }
    return sz;
}

STDMETHODIMP CFileStream::Write(const void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf)
{
    JCHK(NULL != pBuf,E_INVALIDARG);

    if(m_ep == NULL)
        return E_EOF;

    CLocker locker(m_locker);

    ssize_t sz;
    bool eof;
    if(0 != (flag & WRITE_FLAG_FRAME))
    {
        uint32_t count = MAX_MEDIA_FRAME_BUF_COUNT;
        const IMediaFrame::buf* pBufs;
        IMediaFrame* pFrame = (IMediaFrame*)pBuf;
        eof = 0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_EOF);
        JCHK(pBufs = pFrame->GetBuf(0,&count,&szBuf),E_INVALIDARG);
        sz = writev(m_fd,(const iovec*)pBufs,count);
    }
    else
    {
        JCHK(0 < szBuf,E_INVALIDARG);
        eof = 0 != (flag & WRITE_FLAG_EOF);
        sz = write(m_fd,pBuf,szBuf);

    }
    if(0 > sz)
    {
        LOG(0,"%s[%p]:[%s] write fail,error id:%d,message:%s",Class().name,this,m_name.c_str(),errno,strerror(errno));

        return E_FAIL;
    }
    else if(0 == sz)
    {
        LOG(0,"%s[%p]:[%s] write break",Class().name,this,m_name.c_str());

        return E_EOF;
    }
    else
    {
        m_pos += sz;
        m_status.write_total_size += sz;
    }
    if(true == eof)
        return E_EOF;
    else
        return sz;
}

STDMETHODIMP CFileStream::Flush()
{
    JCHK(INVALID_FD != m_fd,E_FAIL);
    //flush(m_fd);
    return S_OK;
}

STDMETHODIMP_(int64_t) CFileStream::Seek(int64_t position,seek_type type)
{
    CLocker locker(m_locker);
    int64_t pos = lseek64(m_fd,position,(int)type);
    if(0 <= pos)
        m_pos = pos;
    return pos;
}

STDMETHODIMP_(int64_t) CFileStream::GetPos()
{
    return m_pos;
}

STDMETHODIMP_(int64_t) CFileStream::GetLength()
{
    CLocker locker(m_locker);
    JCHK(INVALID_FD != m_fd,E_FAIL);
    int64_t len = lseek64(m_fd,0,SEEK_END);
    m_pos = lseek64(m_fd,m_pos,SEEK_SET);
    return len;
}

STDMETHODIMP CFileStream::SetLength(int64_t len)
{
    return E_FAIL;
}

STDMETHODIMP_(bool) CFileStream::IsOpen()
{
    return INVALID_FD != m_fd;
}

STDMETHODIMP_(IStream::status&) CFileStream::GetStatus()
{
    return m_status;
}

STDMETHODIMP CFileStream::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(type == ET_EOF)
    {
        Close();
    }
    else if(type == ET_Error)
    {
        JCHK(INVALID_FD != m_fd,E_FAIL);
        param1 = errno;
        LOG(0,"%s[%p]:[%s] error id:%d message:%s",Class().name,this,m_name.c_str(),errno,strerror(errno));
    }
    else
    {
        CLocker locker(m_locker);
        if(INVALID_FD == m_fd_event)
            return E_AGAIN;

        if(type == ET_Epoll_Output)
        {
            if(m_mode == FT_Source)
            {
                if(0 == m_thread_read)
                    m_thread_read = param1;
                else if(m_thread_read != param1)
                    return E_AGAIN;
                type = ET_Stream_Read;
            }
            else if(m_mode == FT_Render)
                type = ET_Stream_Write;
            else
                return E_FAIL;
        }
        else
            return E_FAIL;
    }
    return NULL == m_callback ? E_FAIL : m_callback->OnEvent((IStream*)this,NULL,type,param1,param2,param3);
}
