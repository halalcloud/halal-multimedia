/////////////////////////////////////////////////////////////////////////////////////////////
// Project:TsFormat
// Author:
// Date:星期三, 三月 7, 2012
// Description:The Cache for Disk I/O
/////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MemoryCache.h"
#include "TsMuxer.h"

CMemoryCache::CMemoryCache()
{
	m_lpCache = NULL;
	m_dwSize = 0;
	m_dwCount = 0;
	_s = NULL;
}

CMemoryCache::~CMemoryCache()
{
	if(m_lpCache)
        delete [] m_lpCache;
}

bool CMemoryCache::SetSize(DWORD dwSize)
{
	m_dwSize = dwSize;
	if(m_lpCache)
        delete [] m_lpCache;
		m_lpCache = new BYTE[m_dwSize];
    return m_lpCache == NULL ? false : true;
}

DWORD CMemoryCache::GetSize()
{
	return m_dwSize;
}

void CMemoryCache::Flush()
{
    if (_s) _s->write(m_lpCache, m_dwCount);
    m_dwCount = 0;
}

DWORD CMemoryCache::Write(LPBYTE lpData,DWORD dwSize)
{
	DWORD dwWrite = 0;
	if(m_dwSize - m_dwCount > dwSize)
	{
        memcpy(&m_lpCache[m_dwCount],lpData,dwSize);
		m_dwCount += dwSize;
		dwWrite += dwSize;
	}
	else
	{
		DWORD dwBytesLeft = dwSize;
		DWORD dwCopyed = m_dwSize - m_dwCount;
		while(dwBytesLeft)
		{
			if(dwCopyed < dwBytesLeft)
			{
			}
			else
			{
				dwCopyed = dwBytesLeft;
			}
            memcpy(&m_lpCache[m_dwCount],&lpData[dwSize - dwBytesLeft],dwCopyed);
			m_dwCount += dwCopyed;
			dwWrite += dwCopyed;
			if(m_dwCount >= m_dwSize)
				Flush();

			dwBytesLeft -= dwCopyed;
		}
	}

	return dwWrite;
}

DWORD CMemoryCache::Read(LPBYTE lpData,DWORD dwSize)
{
	return -1;
}

DWORD CMemoryCache::Seek(DWORD dwNewPos)
{
	return -1;
}

void CMemoryCache::Close()
{
	Flush();
}
