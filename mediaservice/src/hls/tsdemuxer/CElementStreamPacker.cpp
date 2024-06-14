#include "ITSDemuxer.h"
#include <cassert>
#include "CElementStreamPacker.h"

namespace wzd {
	extern f_log		g_log;
CElementStreamPacker::CElementStreamPacker() throw() 
{
	m_uiDataLen = 4 * 1024 * 1024;
	m_data		= new unsigned char[m_uiDataLen];
	memset(m_data, 0, m_uiDataLen);
	memset(&m_pesInfo, 0, sizeof(PESPacket));
	m_usedLen	= 0;
	m_pesLen	= 0;
	m_ownerId	= 0;
	m_id		= 0;
	m_usScramblingMode = 0;
	m_pos		= 0;
}

CElementStreamPacker::~CElementStreamPacker() throw() 
{
	delete [] m_data;
	m_data = NULL;
}

bool CElementStreamPacker::operator < (const CElementStreamPacker& obj) const
{
	if (obj.m_id > this->m_id)
	{
		return false;
	}
	return true;
}
int CElementStreamPacker::GetPESPacket(Packet & pkt, const unsigned char* & pBuf, int iBufLen,
									bool bStartIndicator /*= false*/) throw(IException) 
{
	assert(m_usedLen + 188 < m_uiDataLen);
	if (false == m_usedLen + 188 < m_uiDataLen)
		g_log(NULL, LOG_DEBUG, "CElementStreamPacker::GetPESPacket--->逻辑错误,超出总长度\n");

	int iRetVal = 1;
	if (bStartIndicator)
	{
		//* 非第一个包.
		if (0 == m_pesInfo.packet_start_code_prefix[0] &&
			0 == m_pesInfo.packet_start_code_prefix[1] &&
			1 == m_pesInfo.packet_start_code_prefix[2])
		{			
			__Fill(pkt);
			iRetVal = 0;
		}
		m_esParser.Parser(pBuf, iBufLen);
		m_pesInfo = m_esParser.GetESInfo();
		memcpy(m_data, pBuf, iBufLen);
		m_usedLen += iBufLen;
	}
	else
	{
		//* 头没到,先到了体.
		if (0 != m_pesInfo.packet_start_code_prefix[0] ||
			0 != m_pesInfo.packet_start_code_prefix[1] ||
			1 != m_pesInfo.packet_start_code_prefix[2])
		{
			return iRetVal;

		}
		else//* 直接Copy.
		{
			//* 超出预定义长度.抛出异常.
			if ( m_usedLen + iBufLen >= m_uiDataLen)
			{
				//* 清空数据区.
				memset(&m_pesInfo, 0, sizeof(PESPacket));
				memset(m_data, 0, m_uiDataLen);
				m_usedLen	= 0;
				m_pesLen	= 0;
				throw CMpegtsException("out of memory", 1, "increase predefine memory");
			}
			memcpy(m_data+m_usedLen, pBuf, iBufLen);
			m_usedLen += iBufLen;
		}
	}
	return iRetVal;
}

/**
 * 获取一个包.
 * @param[in]
 * @param[in]
 * @param[out] pkt 一帧数据.
 * @return 组成完整的一帧数据,返回0;反之返回1.
 * @exception IException 出错后抛出异常.
 */
int CElementStreamPacker::GetPacket(Packet & pkt, const unsigned char* & pBuf, int iBufLen,
									bool bStartIndicator /*= false*/) throw(IException) 
{
	assert(m_usedLen + 188 < m_uiDataLen);
	int iRet = 0;
	int iRetVal = 1;
	if (bStartIndicator)
	{
		//* 非第一个包.
		if ((m_pesInfo.PES_packet_length == 0) &&
			0 == m_pesInfo.packet_start_code_prefix[0] &&
			0 == m_pesInfo.packet_start_code_prefix[1] &&
			1 == m_pesInfo.packet_start_code_prefix[2])
		{			
			__Fill(pkt);
			iRetVal = 0;
		}
		iRet = m_esParser.Parser(pBuf, iBufLen);
	}
	int iReserved = 0;
	int iPayload = 0;
	if(iRet)
	{
		//* header
		m_pesInfo = m_esParser.GetESInfo();
		iReserved = m_pesInfo.PES_packet_length;
		iPayload = iBufLen - iRet;
		memcpy(m_data, pBuf+iRet, 
			iReserved>0?(iPayload>iReserved?iReserved:iPayload):iPayload);
		m_usedLen += iPayload;
		//* 实际的PES包的长度.
		m_pesLen = m_pesInfo.PES_packet_length - iRet + 0x6;
	}
	else
	{
		//* 头没到,先到了体.
		if (0 != m_pesInfo.packet_start_code_prefix[0] ||
			0 != m_pesInfo.packet_start_code_prefix[1] ||
			1 != m_pesInfo.packet_start_code_prefix[2])
		{
			return 1;
			//throw CMpegtsException(_T(""), 1, _T(""));
		}

		if (m_pesInfo.PES_packet_length)//* 包长度非0.
		{
			iReserved = m_pesInfo.PES_packet_length - m_usedLen;
			memcpy(m_data+m_usedLen, pBuf, 
				iPayload>iReserved?iReserved:iBufLen);
		}
		else//* 包长度为0,直接Copy.
		{
			memcpy(m_data+m_usedLen, pBuf, iBufLen);
		}
		m_usedLen += iBufLen;
	}

	if (m_pesInfo.PES_packet_length && m_usedLen >= m_pesLen)//* 只有当包长度非0时.
	{
		__Fill(pkt);
		return 0;
	}	
	return iRetVal;
}


/**
 * 获取一个包.
 * @param[in]
 * @param[in]
 * @param[out] pkt 一帧数据.
 * @return 组成完整的一帧数据,返回0;反之返回1.
 * @exception IException 出错后抛出异常.
 */
int CElementStreamPacker::GetPacket(__int64 pos, Packet & pkt, const unsigned char* & pBuf, int iBufLen,
									bool bStartIndicator /*= false*/) throw(IException) 
{
	assert(m_usedLen + 188 < m_uiDataLen);
	int iRet = 0;
	int iRetVal = 1;
	if (bStartIndicator)
	{
		//* 非第一个包.
		if ((m_pesInfo.PES_packet_length == 0) &&
			0 == m_pesInfo.packet_start_code_prefix[0] &&
			0 == m_pesInfo.packet_start_code_prefix[1] &&
			1 == m_pesInfo.packet_start_code_prefix[2])
		{			
			__Fill(pkt);
			iRetVal = 0;
		}
		iRet = m_esParser.Parser(pBuf, iBufLen);
		m_pos = pos;
	}
	int iReserved = 0;
	int iPayload = 0;
	if(iRet)
	{
		//* header
		m_pesInfo = m_esParser.GetESInfo();
		iReserved = m_pesInfo.PES_packet_length;
		iPayload = iBufLen - iRet;
		memcpy(m_data, pBuf+iRet, 
			iReserved>0?(iPayload>iReserved?iReserved:iPayload):iPayload);
		m_usedLen += iPayload;
		//* 实际的PES包的长度.
		m_pesLen = m_pesInfo.PES_packet_length - iRet + 0x6;
	}
	else
	{
		//* 头没到,先到了体.
		if (0 != m_pesInfo.packet_start_code_prefix[0] ||
			0 != m_pesInfo.packet_start_code_prefix[1] ||
			1 != m_pesInfo.packet_start_code_prefix[2])
		{
			return 1;
			//throw CMpegtsException(_T(""), 1, _T(""));
		}

		if (m_pesInfo.PES_packet_length)//* 包长度非0.
		{
			iReserved = m_pesInfo.PES_packet_length - m_usedLen;
			memcpy(m_data+m_usedLen, pBuf, 
				iPayload>iReserved?iReserved:iBufLen);
		}
		else//* 包长度为0,直接Copy.
		{
			memcpy(m_data+m_usedLen, pBuf, iBufLen);
		}
		m_usedLen += iBufLen;
	}

	if (m_pesInfo.PES_packet_length && m_usedLen >= m_pesLen)//* 只有当包长度非0时.
	{
		__Fill(pkt);
		iRetVal = 0;
	}	
	return iRetVal;
}
bool CElementStreamPacker::Flush(Packet& pkt)
{
	bool bRet = __Fill(pkt);
	memset(m_data, 0, m_uiDataLen);
	memset(&m_pesInfo, 0, sizeof(PESPacket));
	return bRet;
}

void CElementStreamPacker::Reset()
{
	memset(&m_pesInfo, 0, sizeof(PESPacket));
	memset(m_data, 0, m_uiDataLen);
	m_usedLen = 0;
}

bool CElementStreamPacker::__Fill(Packet& pkt)
{
	bool bRet	= false;
	//* 对于分配0字节的数组,我感觉是不应该的.
	//* 所以加这个判断.
	//* 指向0字节数组的指针是有值的.我不清楚Delete它会怎么样,
	//* 我说不准所以加了判断.
	if (m_usedLen > 0)
	{
		pkt.pbuf	= new unsigned char[m_usedLen];
	}
	pkt.len		= m_usedLen;
	pkt.pid		= m_id;
	pkt.pts		= m_pesInfo.PTS;
	pkt.dts		= m_pesInfo.DTS;
	pkt.usScramblingMode = m_usScramblingMode;
	//* pts
	if (m_pesInfo.PTS_DTS_flags == 0x2)
	{
		pkt.dts = pkt.pts;
	}
	
	memcpy(pkt.pbuf, m_data, pkt.len);
	if (0 == m_pesInfo.packet_start_code_prefix[0] &&
		0 == m_pesInfo.packet_start_code_prefix[1] &&
		1 == m_pesInfo.packet_start_code_prefix[2] )
	{
		bRet = true;
	}
	pkt.pos = m_pos;
	//* 清空数据区.
	memset(&m_pesInfo, 0, sizeof(PESPacket));
	//* 没想到它居然是最花费时间的地方.
	//memset(m_data, 0, m_uiDataLen);
	m_usedLen	= 0;
	m_pesLen	= 0;
	m_pos		= 0;
	return bRet;
}


} // namespace wzd
