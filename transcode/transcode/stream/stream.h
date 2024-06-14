#ifndef STREAM_H
#define STREAM_H

#include "stdafx.h"

class CStream
{
public:
    CStream();
    virtual ~CStream();
    void Close();
    HRESULT Write(IBuffer* pBuf);
    HRESULT Read(uint8_t* pBuf,int32_t szBuf);
    HRESULT Write(uint8_t* pBuf,int32_t szBuf);
    int64_t Seek(const int64_t& position,IStream::seek_type type);
public:
    int m_fd;
    string m_name;
};

#endif // STREAM_H
