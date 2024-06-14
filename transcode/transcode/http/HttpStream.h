#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include "stdafx.h"

class CHttpStream : public IStream , public IEventCallbackSet , public IEventCallback
{
public:
    DOM_DECLARE(CHttpStream)
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
    //CHttpSession
protected:
    dom_ptr<IStream> m_tcp;
    IEventCallback* m_callback;
    void* m_tag;
    uint32_t m_mode;
};

#endif // HTTPSTREAM_H
