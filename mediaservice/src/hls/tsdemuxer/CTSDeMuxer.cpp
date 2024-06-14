
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
	* �����ݽ��з���.
	* �ڻ�ȡ��һ֡ESʱ����.
	* ���������Ҫ��֯�����Ҫ������.(�����ǽ�TS,PSI,ES�����ݺϲ���һ���ṹ)
	* @param[out] һ֡����
	* @param[out] �����ı���Ϣ.
	* @param[in] sync �Ƿ�����ȡ����֡
	* @return ��������Ϣ.
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

		//* ������������
		if (tsInfo.transportScramblingControl != 0x0)
		{
			pkt.pid					= tsInfo.pid;
			pkt.usScramblingMode	= tsInfo.transportScramblingControl;
			g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetPESPacket--->%s", "������������\n");
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
					// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
					// ���ü���ģʽ
					m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
					// }} */
					if (m_tsParser.IsLostPackage())
					{
						pkt.bContinue = false;
						//* ����Ϊ0����Ϊ����.
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
				// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
				// ���ü���ģʽ
				m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
				// }} */
				if (m_tsParser.IsLostPackage())
				{
					if (true == m_packers[pid]->Flush(pkt))
						iRet = 0;
					pkt.bContinue = false;
					g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetPESPacket--->��⵽����, pid: %i\n", pid);
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

		//* ������������
		if (tsInfo.transportScramblingControl != 0x0)
		{
			pkt.pid					= tsInfo.pid;
			pkt.usScramblingMode	= tsInfo.transportScramblingControl;
			g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetESPacket--->%s", "������������\n");
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
					// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
					// ���ü���ģʽ
					m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
					// }} */
					//* �������
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
				// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
				// ���ü���ģʽ
				m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
				// }} */
				if (m_tsParser.IsLostPackage())
				{
					g_log(NULL, LOG_DEBUG,"CTSDeMuxer::GetESPacket--->��⵽����, pid: %i\n", pid);
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
	* �����ݽ��з���.
	* �ڻ�ȡ��һ֡ESʱ����.
	* @exception ����ʧ���׳��쳣.
	* @param[out] һ֡����
	* @param[in] buf ������ָ��.�̶�188,192,204,208
	* @return ��ȡ������֡����0,�����з���1.
	* @note ���ڼ�����,����pkt.transportScramblingControl��
	*/
	int CTSDeMuxer::GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen /*= 188*/) throw(IException) 
	{
		int iRet = 1;
		unsigned short pid;
		int iOffset;
		iOffset = m_tsParser.Parser(buf, iLen, pid);
		TransportPacket tsInfo;
		m_tsParser.GetTSInfo(tsInfo);

		//* ������������
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
					// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
					// ���ü���ģʽ
					m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
					// }} */
					//* �������
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
				// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
				// ���ü���ģʽ
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
	* �������ڲ�״̬�ָ�����ʼ��״̬.
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
	* ��ʼ��.
	* ����TS����ı�������������.
	*/
	void CTSDeMuxer::Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw() 
	{

	}

	/**
	* ��ʼ��.
	* ����TS����ı�������������.
	* @exception IException
	* @param[in] pBuf ���ݰ�ָ��.
	* @param[in] ���ݰ���С.188,192,204,208
	* @param[in] pid ���ݰ���PIDֵ.
	* @return �����ɹ�����00,�����з���1.
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
		//* �����˳�����
		if (iRet != 0)
		{
			//* �ۼӰ�������.����1W������û�гɹ�.���ж��������.
			++m_iPkeCnt;
			if (m_iPkeCnt > PacketCountThreshold)
			{
				m_iPkeCnt = 0;
				iRet = __CreatePackers();
			}
		}

		//* ����PCRͳ������.
		if (m_iPCRCnt > 0 && m_iPCRCnt < 2)
		{
			m_iCalcBitRate += 188;
		}
		if (true == tsInfo.AdaptationField.PCR_flag
			&& m_iPCRCnt < 2)
		{
			//* �����һ��PCR����.
			if (0 == m_iPCRBase && 0 == m_iPCRExt)
			{
				m_iPCRBase = tsInfo.AdaptationField.program_clock_reference_base;
				m_iPCRExt = tsInfo.AdaptationField.program_clock_reference_extension;
			}
			else//* ����PCR��ֵ
			{
				m_iCalcBitRate = m_iCalcBitRate * 27000000
					/ (tsInfo.AdaptationField.program_clock_reference_base * 300 +
					tsInfo.AdaptationField.program_clock_reference_extension 
					- (m_iPCRBase * 300 + m_iPCRExt));
			}
			++m_iPCRCnt;
		}

		if (0 == iRet && m_iPCRCnt >=2)//* ����ͳ�ƺ�PSI������Ҫ���.
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
	* �ж��Ƿ����Seek
	* @return ���Է�����,��֮���ؼ���.
	*/
	bool CTSDeMuxer::IsSeekAble() throw() 
	{
		return false;

	}

	/**
	* ������Դ.
	* @param[in] pkt �����յ���Դָ��.
	* @return ��.
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
	* ��ȡ��Ŀ��.
	* @return ���򷵻ؾ����Ŀ��Ŀ,û�з���0.�����ظ�ֵ.
	*/
	int CTSDeMuxer::GetProgramCount() throw() 
	{

		return 0;

	}

	/**
	* ��ʼ���ڲ�����.
	*/
	void CTSDeMuxer::__Init() throw(IException) 
	{

	}

	/**
	* ��ȡһ����,������.
	* ������TS�����������.��:PES����SI��Ϣ.
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
				//* �����������.
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

		//* ������������
		if (tsInfo.transportScramblingControl == 0x0)
		{
			//* ����PSI�ͷ�PESͷ.
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
		// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
		// ���ü���ģʽ
		m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
		// }} */

		//* ����Ϊ0����Ϊ����.
		m_packers[m_uiFlushCnt]->Flush(pkt);
		++m_uiFlushCnt;
#else
		//* 
		/* {{ delete by wzduo 2012-03-01
		// ���ڼ���������Զȡ����PESͷ,����ֱ����TS��������.
		// ���ü���ģʽ
		m_packers[i]->SetScramblingMode(tsInfo.transportScramblingControl);
		// }} */
		//* �������ֵ,�˳�
		//if (m_iFlushCnt >= std::numeric_limits<int>::max())
		//{
		//	return false;
		//}
		////* �ҵ���ȷ�Ķ���λ��.
		//while (!m_packers[m_iFlushCnt])
		//{
		//	++m_iFlushCnt;
		//}
		////* ��ȡ����,ָ����һλ��.
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
