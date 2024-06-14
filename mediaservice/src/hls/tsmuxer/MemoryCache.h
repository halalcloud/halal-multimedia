/////////////////////////////////////////////////////////////////////////////////////////////
// Project:TsFormat
// Author:
// Date:星期三, 三月 7, 2012
// Description:The Cache for Disk I/O
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __MEMORYCACHE_H__
#define __MEMORYCACHE_H__

#include "stdafx.h"
#include "libmpeg_sink.h"
#define MAX_URI_LENGTH 256

class CMemoryCache
{
public:
	CMemoryCache();
	~CMemoryCache();
    void SetSink(muxer_sink* s) { _s = s; }
    bool SetSize(DWORD dwSize);
	DWORD GetSize();
    void Flush();
	DWORD Write(LPBYTE lpData,DWORD dwSize);
	DWORD Read(LPBYTE lpData,DWORD dwSize);
	DWORD Seek(DWORD dwNewPos);
    void Close();
private:
	LPBYTE m_lpCache;
	DWORD m_dwSize;
	DWORD m_dwCount;
    muxer_sink* _s;
};

#endif // __MEMORYCACHE_H__
