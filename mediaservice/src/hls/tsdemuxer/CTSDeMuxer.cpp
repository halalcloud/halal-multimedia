
#include <cassert>
#include "CTSDeMuxer.h"
#include <climits>
#include <cwchar>
#include <limits>
#include <stdio.h>
#include <stdarg.h>
using namespace std;
namespace wzd {
	f_log		g_log;
	static void wzd_log_default( void *p_unused, int i_level, const char *psz_fmt, va_list arg )
	{
		char *psz_prefix;
		switch( i_level )
		{
		case LOG_ERROR:
			psz_prefix = "error";
			break;
		case LOG_WARNING:
			psz_prefix = "warning";
			break;
		case LOG_INFO:
			psz_prefix = "info";
			break;
		case LOG_DEBUG:
			psz_prefix = "debug";
			break;
		default:
			psz_prefix = "unknown";
			break;
		}
		fprintf( stderr, "mpegts [%s]: ", psz_prefix );
		vfprintf( stderr, psz_fmt, arg );
	}

	void wzd_log(void * unused, int i_level, const char *psz_fmt, ... )
	{
		va_list arg;
		va_start( arg, psz_fmt );
		wzd_log_default( unused, i_level, psz_fmt, arg );
		va_end( arg );
	}

	CTSDeMuxer::CTSDeMuxer() throw() 
	{
		m_iPkeCnt = 0;
		m_iPCRCnt = 0;
		m_iCalcBitRate = 0;
		m_iPCRBase = 0;
		m_iPCRExt = 0;
		m_strmProvider = NULL;
		g_log = wzd_log;
#ifndef __use_array__
#else
		m_packers = new CElementStreamPacker*[65536];
		memset(m_packers, 0, sizeof(int*)*65536);
#endif
	}

	CTSDeMuxer::~CTSDeMuxer() throw() 
	{
		__ReleasePacker();
	}


	/**
	* 对数据进行分析.
	* 在获取到一帧ES时返回.
	* 这里根据需要组织输出需要的数据.(基本是将TS,PSI,ES的数据合并成一个结构)
	* @param[out] 一帧数据
	* @param[out] 错误文本信息.
	* @param[in] sync 是否必须获取完整帧
	* @return 错误码信息.
	*/
	int CTSDeMuxer::GetESPacket(Packet & pkt, _tstring & strErrInfo, bool sync) throw() 
	{
        assert(0);
		return 0;

	}

	int CTSDeMuxer::GetPESPacket(Packet & pkt, const unsigned char* & buf, int iLen /*= 188*/) throw(IException)
	{

		int iRet = 1;
		unsigned short pid;
		int iOffset;
		iOffset = m_tsParser.Parser(buf, iLen, pid);
		TransportPacket tsInfo;
		m_tsParser.GetTSInfo(tsInfo);

		//* 跳过加密数据
		if (tsInfo.transportScramblingControl != 0x0)
		{
			pkt.pid					= tsInfo.pid;
			pkt.usScramblingMode	= tsInfo.transportScramblingControl;
			g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetPESPacket--->%s", "跳过加密数据\n");
			return 0;
		}

		//if (0x0 != pid && !m_psiParser.IsPSI(pid))
		{
#ifndef __use_array__
			//* 
			for (unsigned int i = 0; i < m_packers.size(); ++i)
			{
				if (pid == m_packers[i]->GetID())
				{
					/* {{ delete by wzduo 2012-03-01
					// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
					// 设置加扰模式
					m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
					// }} */
					if (m_tsParser.IsLostPackage())
					{
						pkt.bContinue = false;
						//* 均不为0才认为正常.
						if (true == m_packers[i]->Flush(pkt))
							iRet = 0;
					}
					else
					{
						const unsigned char* pBuf = buf + iOffset;
						iRet = m_packers[i]->GetPESPacket(pkt, pBuf, iLen - iOffset, tsInfo.payloadUnitStartIndicator);
						pkt.bContinue = true;
					}
					break;
				}
			}
#else
			//* 
			if (m_packers[pid] != 0)
			{
				/* {{ delete by wzduo 2012-03-01
				// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
				// 设置加扰模式
				m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
				// }} */
				if (m_tsParser.IsLostPackage())
				{
					if (true == m_packers[pid]->Flush(pkt))
						iRet = 0;
					pkt.bContinue = false;
					g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetPESPacket--->检测到丢包, pid: %i\n", pid);
				}
				else
				{
					const unsigned char* pBuf = buf + iOffset;
                    //g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetPESPacket--->GetPESPacket[%d] in\n", pid);
					iRet = m_packers[pid]->GetPESPacket(pkt, pBuf, iLen - iOffset, tsInfo.payloadUnitStartIndicator);
					pkt.bContinue = true;
                    //g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetPESPacket--->GetPESPacket[%d] out\n", pid);
				}
			}
#endif
		}

		return iRet;
	}

	int CTSDeMuxer::GetESPacket(__int64 pos, Packet & pkt, const unsigned char* & buf, int iLen /*= 188*/) throw(IException)
	{
		int iRet = 1;
		unsigned short pid;
		int iOffset;
		iOffset = m_tsParser.Parser(buf, iLen, pid);
		TransportPacket tsInfo;
		m_tsParser.GetTSInfo(tsInfo);

		//* 跳过加密数据
		if (tsInfo.transportScramblingControl != 0x0)
		{
			pkt.pid					= tsInfo.pid;
			pkt.usScramblingMode	= tsInfo.transportScramblingControl;
			g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetESPacket--->%s", "跳过加密数据\n");
			return 0;
		}

		//if (0x0 != pid && !m_psiParser.IsPSI(pid))
		{
#ifndef __use_array__
			//* 
			for (unsigned int i = 0; i < m_packers.size(); ++i)
			{
				if (pid == m_packers[i]->GetID())
				{
					/* {{ delete by wzduo 2012-03-01
					// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
					// 设置加扰模式
					m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
					// }} */
					//* 丢包检测
					if (m_tsParser.IsLostPackage())
					{
						pkt.bContinue = false;
						if (true == m_packers[i]->Flush(pkt))
							iRet = 0;
					}
					else
					{
						const unsigned char* pBuf = buf + iOffset;
						iRet = m_packers[i]->GetPacket(pos, pkt, pBuf, iLen - iOffset, tsInfo.payloadUnitStartIndicator);
						pkt.bContinue = true;
					}
					break;
				}
			}
#else
			//* 
			if (m_packers[pid] != 0)
			{
				/* {{ delete by wzduo 2012-03-01
				// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
				// 设置加扰模式
				m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
				// }} */
				if (m_tsParser.IsLostPackage())
				{
					g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetESPacket--->检测到丢包, pid: %i\n", pid);
					if (true == m_packers[pid]->Flush(pkt))
						iRet = 0;
					pkt.bContinue = false;
				}
				else
				{
					const unsigned char* pBuf = buf + iOffset;
                    //g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetESPacket--->GetPacket[%d] in\n", pid);
					iRet = m_packers[pid]->GetPacket(pos, pkt, pBuf, iLen - iOffset, tsInfo.payloadUnitStartIndicator);
					pkt.bContinue = true;
                    //g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetESPacket--->GetPacket[%d] out\n", pid);
				}
			}
#endif
		}
		return iRet;
	}
	/**
	* 对数据进行分析.
	* 在获取到一帧ES时返回.
	* @exception 处理失败抛出异常.
	* @param[out] 一帧数据
	* @param[in] buf 数据区指针.固定188,192,204,208
	* @return 获取到完整帧返回0,正理中返回1.
	* @note 对于加扰流,仅置pkt.transportScramblingControl项
	*/
	int CTSDeMuxer::GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen /*= 188*/) throw(IException) 
	{
		int iRet = 1;
		unsigned short pid;
		int iOffset;
		iOffset = m_tsParser.Parser(buf, iLen, pid);
		TransportPacket tsInfo;
		m_tsParser.GetTSInfo(tsInfo);

		//* 跳过加密数据
		if (tsInfo.transportScramblingControl != 0x0)
		{
			pkt.pid					= tsInfo.pid;
			pkt.usScramblingMode	= tsInfo.transportScramblingControl;
			return 0;
		}

		//if (0x0 != pid && !m_psiParser.IsPSI(pid))
		{
#ifndef __use_array__
			//* 
			for (unsigned int i = 0; i < m_packers.size(); ++i)
			{
				if (pid == m_packers[i]->GetID())
				{
					/* {{ delete by wzduo 2012-03-01
					// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
					// 设置加扰模式
					m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
					// }} */
					//* 丢包检测
					if (m_tsParser.IsLostPackage())
					{
						pkt.bContinue = false;
						if (true == m_packers[i]->Flush(pkt))
							iRet = 0;
					}
					else
					{
						const unsigned char* pBuf = buf + iOffset;
						iRet = m_packers[i]->GetPacket(pkt, pBuf, iLen - iOffset, tsInfo.payloadUnitStartIndicator);
						pkt.bContinue = true;
					}
					break;
				}
			}
#else
			//* 
			if (m_packers[pid] != 0)
			{
				/* {{ delete by wzduo 2012-03-01
				// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
				// 设置加扰模式
				m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
				// }} */
				if (m_tsParser.IsLostPackage())
				{
					if (true == m_packers[pid]->Flush(pkt))
						iRet = 0;
					pkt.bContinue = false;
				}
				else
				{
					const unsigned char* pBuf = buf + iOffset;
					iRet = m_packers[pid]->GetPacket(pkt, pBuf, iLen - iOffset, tsInfo.payloadUnitStartIndicator);
					pkt.bContinue = true;
				}
			}
#endif
		}
		return iRet;
	}

	/**
	* 将对象内部状态恢复到初始化状态.
	*/
	void CTSDeMuxer::Reset() throw() 
	{
#ifndef __use_array__
		for (unsigned int i = 0; i < m_packers.size(); ++i)
		{
			m_packers[i]->Reset();
		}
#else
		for (unsigned int i = 0; i < 65536; ++i)
		{
			if (0 != m_packers[i]) m_packers[i]->Reset();
		}
#endif
		m_tsParser.ResetCounter();
	}

	/**
	* 初始化.
	* 构造TS所需的表必须在这里完成.
	*/
	void CTSDeMuxer::Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw() 
	{

	}

	/**
	* 初始化.
	* 构造TS所需的表必须在这里完成.
	* @exception IException
	* @param[in] pBuf 数据包指针.
	* @param[in] 数据包大小.188,192,204,208
	* @param[in] pid 数据包的PID值.
	* @return 解析成功返回00,解析中返回1.
	*/
	int CTSDeMuxer::Init(const unsigned char* & buf, int iLen /*= 188*/) throw(IException) 
	{
		int iRet = 1;
		unsigned short pid;
		int iOffset;
		iOffset = m_tsParser.Parser(buf, iLen, pid);
		TransportPacket tsInfo;
		m_tsParser.GetTSInfo(tsInfo);
		if (pid == 0x0 || m_psiParser.IsPSI(pid))
		{
			if (tsInfo.payloadUnitStartIndicator == 1)
			{
				++iOffset;
			}
			const unsigned char* pTmp = buf + iOffset;
			if (0 == m_psiParser.Parser(pTmp, iLen - iOffset, pid))
			{
				iRet = __CreatePackers();
			}

		}
		//* 意外退出条件
		if (iRet != 0)
		{
			//* 累加包计数器.解析1W个包还没有成功.则有多少算多少.
			++m_iPkeCnt;
			if (m_iPkeCnt > PacketCountThreshold)
			{
				m_iPkeCnt = 0;
				iRet = __CreatePackers();
			}
		}

		//* 根据PCR统计码率.
		if (m_iPCRCnt > 0 && m_iPCRCnt < 2)
		{
			m_iCalcBitRate += 188;
		}
		if (true == tsInfo.AdaptationField.PCR_flag
			&& m_iPCRCnt < 2)
		{
			//* 保存第一个PCR数据.
			if (0 == m_iPCRBase && 0 == m_iPCRExt)
			{
				m_iPCRBase = tsInfo.AdaptationField.program_clock_reference_base;
				m_iPCRExt = tsInfo.AdaptationField.program_clock_reference_extension;
			}
			else//* 计算PCR差值
			{
				m_iCalcBitRate = m_iCalcBitRate * 27000000
					/ (tsInfo.AdaptationField.program_clock_reference_base * 300 +
					tsInfo.AdaptationField.program_clock_reference_extension 
					- (m_iPCRBase * 300 + m_iPCRExt));
			}
			++m_iPCRCnt;
		}

		if (0 == iRet && m_iPCRCnt >=2)//* 码率统计和PSI分析都要完成.
		{
			iRet = 0;
		}
		else
		{
			iRet = 1;
		}
		return iRet;
	}

	/**
	* 判断是否可以Seek
	* @return 可以返回真,反之返回假如.
	*/
	bool CTSDeMuxer::IsSeekAble() throw() 
	{
		return false;

	}

	/**
	* 回收资源.
	* @param[in] pkt 被回收的资源指针.
	* @return 空.
	*/
	void CTSDeMuxer::RecyclePacket(Packet & pkt) throw() 
	{
		delete [] pkt.pbuf;
		pkt.pbuf = NULL;
		pkt.len = 0;
		pkt.pid = 0;
	}

	int CTSDeMuxer::GetProgramNames(vector<_tstring> & strnames) throw() 
	{
		return 0;

	}

	/**
	* 获取节目数.
	* @return 有则返回具体节目数目,没有返回0.出错返回负值.
	*/
	int CTSDeMuxer::GetProgramCount() throw() 
	{

		return 0;

	}

	/**
	* 初始化内部环境.
	*/
	void CTSDeMuxer::__Init() throw(IException) 
	{

	}

	/**
	* 读取一个包,供分析.
	* 它分析TS层的数据类型.即:PES还是SI信息.
	*/
	void CTSDeMuxer::__ReadPacket() throw(IException) 
	{

	}

#ifndef __use_array__
	void CTSDeMuxer::__AddPacker(unsigned short pid, unsigned short ownerID)
	{
		for (unsigned int i = 0; i < m_packers.size(); ++i)
		{
			if (m_packers[i]->GetID() == pid)
			{
				return;
			}
		}

		CElementStreamPacker* packer = new CElementStreamPacker;
		packer->SetID(pid);
		packer->SetOwnerID(ownerID);
		m_packers.push_back(packer);
	}

	void CTSDeMuxer::__ReleasePacker()
	{
		for (unsigned int i = 0; i < m_packers.size(); ++i)
		{
			delete m_packers[i];
			m_packers[i] = NULL;
		}

	}
#else
	void CTSDeMuxer::__AddPacker(unsigned short pid, unsigned short ownerID)
	{

		if (m_packers[pid] == 0)
		{
			CElementStreamPacker* packer = new CElementStreamPacker;
			packer->SetID(pid);
			packer->SetOwnerID(ownerID);
			m_packers[pid] = packer;
		}
	}

	void CTSDeMuxer::__ReleasePacker()
	{
		for (int i = 0; i < 65536; ++i)
		{
			delete m_packers[i];
			m_packers[i] = NULL;
		}
		delete [] m_packers;
		m_packers = NULL;
	}
#endif


	int  CTSDeMuxer::__CreatePackers()
	{
		const vector<ProgramMapSection>& pmts =  m_psiParser.GetPMT();
		for (unsigned int i = 0; i < pmts.size(); ++i)
		{
			int iPid, iOwner;
			iOwner = pmts[i].program_number;
			for(unsigned int j = 0; j < pmts[i].esInfo.size(); ++j)
			{
				iPid =  pmts[i].esInfo[j].elementary_PID;
				//* 增加流解包器.
				__AddPacker(iPid, iOwner);
			}
		}
		return 0;
	}

	CTSDeMuxer2::CTSDeMuxer2(f_log log /*= NULL*/)
	{
		if (NULL != log)
			g_log = log;
		m_uiFlushCnt = 0;
	}

	CTSDeMuxer2::~CTSDeMuxer2() throw()
	{
		m_uiFlushCnt = 0;
	}

	bool CTSDeMuxer2::GetTimeStamp(unsigned short& pid, long long & pts, 
		long long& dts, const unsigned char* buf, 
		int iLen /*= 188*/) throw()
	{
		pid = 0;
		pts = 0;
		dts = 0;
		CTransportStreamParser tsParser;
		int iOffset;
		iOffset = tsParser.Parser(buf, iLen, pid);
		TransportPacket tsInfo;
		tsParser.GetTSInfo(tsInfo);

		//* 跳过加密数据
		if (tsInfo.transportScramblingControl == 0x0)
		{
			//* 跳过PSI和非PES头.
			if (pid != 0 && !m_psiParser.IsPSI(pid) &&
				tsInfo.payloadUnitStartIndicator)
			{
				const unsigned char* pBuf = buf + iOffset;
				CElementStreamParser esParser;
				if (0 != esParser.Parser(pBuf, iLen - iOffset))
				{
					PESPacket esInfo = esParser.GetESInfo();
					pts = esInfo.PTS;
					dts = esInfo.DTS;
					pid = tsInfo.pid;
					return true;
				}
			}
		}
		return false;
	}

	bool CTSDeMuxer2::Flush(Packet& pkt)
	{
#ifndef __use_array__
		//* 
		if (m_uiFlushCnt >= m_packers.size())
		{
			return false;
		}

		/* {{ delete by wzduo 2012-03-01
		// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
		// 设置加扰模式
		m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
		// }} */

		//* 均不为0才认为正常.
		m_packers[m_uiFlushCnt]->Flush(pkt);
		++m_uiFlushCnt;
#else
		//* 
		/* {{ delete by wzduo 2012-03-01
		// 由于加密数据永远取不到PES头,所以直接在TS层面跳过.
		// 设置加扰模式
		m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
		// }} */
		//* 超出最大值,退出
		//if (m_iFlushCnt >= std::numeric_limits<int>::max())
		//{
		//	return false;
		//}
		////* 找到正确的对象位置.
		//while (!m_packers[m_iFlushCnt])
		//{
		//	++m_iFlushCnt;
		//}
		////* 读取数据,指向下一位置.
		//m_packers[m_iFlushCnt]->Flush(pkt);
		//++m_iFlushCnt;
#endif
		return true;
	}

	bool CTSDeMuxer2::GetPCR( long long& pcr, unsigned short& pid,
		const unsigned char* buf, int iLen /*= 188*/)
	{
		pcr = 0;
		pid = 0;
		CTransportStreamParser tsParser;
		int iOffset;
		iOffset = tsParser.Parser(buf, iLen, pid);
		TransportPacket tsInfo;
		tsParser.GetTSInfo(tsInfo);
		if (1 == tsInfo.AdaptationField.PCR_flag)
		{
			pcr = tsInfo.AdaptationField.program_clock_reference_base * 300 +
				tsInfo.AdaptationField.program_clock_reference_extension;
			return true;
		}
		return false;
	}
} // namespace wzd
