#ifndef FILE_H
#define FILE_H

#include "stream.h"

class CFileStream : public IStream , public IEventCallback
{
public:
    DOM_DECLARE(CFileStream)
    //IStream
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP Open(url* pUrl,int mode);
    STDMETHODIMP_(void) Close();
    STDMETHODIMP Read(void* pBuf,int32_t szBuf);
    STDMETHODIMP Write(void* pBuf,int32_t szBuf);
    STDMETHODIMP_(int64_t) Seek(int64_t position,seek_type type);
    STDMETHODIMP_(int64_t) GetLength();
    STDMETHODIMP SetLength(int64_t len);
    STDMETHODIMP_(bool) IsOpen();
    //IEventCallbackSet
    STDMETHODIMP_(void) SetCallback(IEventCallback* pCallback,void* pTag);
    //IEventCallback
    STDMETHODIMP OnEvent(uint32_t id,void* pParam,void* pTag);
    //CTcpStream
protected:
    CStream m_stream;
    void* m_tag;
};

#endif // FILE_H
