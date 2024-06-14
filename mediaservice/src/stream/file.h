#ifndef FILE_H
#define FILE_H

#include "stdafx.h"

class CFileStream : public IStream, public ICallback
{
public:
    DOM_DECLARE(CFileStream)
    //IStream
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP Open(const char*  pUrl,uint32_t mode);
    STDMETHODIMP_(void) Close();
    STDMETHODIMP SetEventEnable(bool enable);
    STDMETHODIMP_(bool) GetEventEnable();
    STDMETHODIMP SetTimer(int id,uint32_t duration,bool one_shoot);
    STDMETHODIMP_(bool) CanRead();
    STDMETHODIMP_(bool) CanWrite();
    STDMETHODIMP Read(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf);
    STDMETHODIMP Write(const void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf);
    STDMETHODIMP Flush();
    STDMETHODIMP_(int64_t) Seek(int64_t position,seek_type type);
    STDMETHODIMP_(int64_t) GetPos();
    STDMETHODIMP_(int64_t) GetLength();
    STDMETHODIMP SetLength(int64_t len);
    STDMETHODIMP_(bool) IsOpen();
    STDMETHODIMP_(status&) GetStatus();
    //IEventCallback
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3);
    //CFileStream
    HRESULT Close(HRESULT hr);
protected:
    ICallback* m_callback;
    int m_fd;
    int m_fd_event;
    uint32_t m_mode;
    uint64_t m_msg_event;
    string m_name;
    int64_t m_pos;
    status m_status;
    uint32_t m_flag;
    uint32_t m_timer;
    dom_ptr<IEpollPoint> m_ep;
    ILocker* m_locker;
    int m_thread_read;
};

#endif // FILE_H
