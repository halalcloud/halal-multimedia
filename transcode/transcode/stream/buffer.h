#ifndef BUFFER_H
#define BUFFER_H

#include "stdafx.h"

class CBuffer : public IBuffer , public IStream
{
public:
    DOM_DECLARE(CBuffer)
    //IBuffer
    STDMETHODIMP Alloc(uint32_t size);
    STDMETHODIMP_(uint32_t) GetSize();
    STDMETHODIMP_(void) Clear();
    STDMETHODIMP CopyFrom(IBuffer* pBuf);
    STDMETHODIMP CopyTo(IBuffer* pBuf);
    //IStream
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP Open(url* pUrl,int mode);
    STDMETHODIMP_(void) Close();
    STDMETHODIMP Read(void* pBuf,int szBuf);
    STDMETHODIMP Write(void* pBuf,int szBuf);
    STDMETHODIMP_(int64_t) Seek(int64_t position,seek_type type);
    STDMETHODIMP_(int64_t) GetLength();
    STDMETHODIMP SetLength(int64_t len);
    STDMETHODIMP_(bool) IsOpen();
    //CBuffer
    HRESULT Write(ISerialize* pObj);
    static HRESULT Copy(IBuffer* pDest,IBuffer* pSour);
protected:
    uint8_t* m_data;
    uint32_t m_size;
    uint32_t m_pos_write;
    uint32_t m_pos_read;
};

#endif // BUFFER_H
