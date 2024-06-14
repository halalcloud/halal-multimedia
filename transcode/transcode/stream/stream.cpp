#include "stream.h"

CStream::CStream()
:m_fd(INVALID_FD)
{
    //ctor
}

CStream::~CStream()
{
    Close();
}

void CStream::Close()
{
    if(INVALID_FD != m_fd)
    {
        close(m_fd);
        LOG(0,"host:%s[%d] close\n",m_name.c_str(),m_fd);
        m_fd = INVALID_FD;
    }
}

HRESULT CStream::Read(uint8_t* pBuf,int32_t szBuf)
{
    JCHK(NULL != pBuf,E_INVALIDARG);
    JCHK(0 < szBuf,E_INVALIDARG);
    JCHK(INVALID_FD != m_fd,E_FAIL);

    ssize_t sz = read(m_fd,pBuf,szBuf);

    if(0 > sz)
    {
        if(EINTR == errno || EAGAIN == errno || EWOULDBLOCK == errno)
            return E_AGAIN;
        else
        {
            JCHK4(false,E_FAIL,
                "host:%s[%d] read fail,error id:%d,message:%s",
                m_name.c_str(),m_fd,errno,strerror(errno));
        }
    }
    else if(0 == sz)
    {
        int fd = m_fd;
        LOG(1,"host:%s[%d] read break connect",m_name.c_str(),fd);
        return E_EOF;
    }
    return sz;
}

HRESULT CStream::Write(uint8_t* pBuf,int32_t szBuf)
{
    JCHK(NULL != pBuf,E_INVALIDARG);
    JCHK(0 < szBuf,E_INVALIDARG);
    JCHK(INVALID_FD != m_fd,E_FAIL);

    ssize_t sz = write(m_fd,pBuf,szBuf);

    if(0 > sz)
    {
        if(EINTR == errno || EAGAIN == errno || EWOULDBLOCK == errno)
            return E_AGAIN;
        else
        {
            JCHK4(false,E_FAIL,
                "host:%s[%d] write fail,error id:%d,message:%s",
                m_name.c_str(),m_fd,errno,strerror(errno));
        }
    }
    else if(0 == sz)
    {
        LOG(1,"host:%s[%d] write break connect",m_name.c_str(),m_fd);
        return E_EOF;
    }
    return sz;
}

int64_t CStream::Seek(const int64_t& position,IStream::seek_type type)
{
    int64_t result;
    JCHK2(-1 != (result = lseek64(m_fd,position,type)),E_FAIL,
        "seek fail,error id:%d message:%s",errno,strerror(errno));
    return result;
}

