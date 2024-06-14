#include <cassert>
#include "CTransportStreamParser.h"
#include "wzdBits.h"
using namespace std;
namespace wzd
{

CTransportStreamParser::CTransportStreamParser() throw ()
{
	m_bContinuity = true;
	memset(&m_tsPkt, 0, sizeof(TransportPacket));
#ifndef __use_array__
#else
	m_counterCnt	= 0;
	m_counter		= NULL;
#endif
}

/**
 * 构造函数
 */
CTransportStreamParser::CTransportStreamParser(const void* & userData) throw ()
{
	this->m_bContinuity = false;
	memset(&m_tsPkt, 0, sizeof(TransportPacket));
#ifndef __use_array__
#else
	m_counterCnt	= 0;
	m_counter		= NULL;
#endif
}

CTransportStreamParser::~CTransportStreamParser()
{
#ifndef __use_array__
#else
	if (m_counter)
	{
		delete [] m_counter;
		m_counter = NULL;
	}
	m_counterCnt = 0;
#endif
}
/**
 * 解析字节流.
 * 一次必须是一个TS包的大小.
 * @exception IException
 * @param[in] pBuf 数据包指针.
 * @param[in] 数据包大小.
 * @param[out] pid 本数据包的PID.
 * @return 解析失败返回0,解析成功返回数据偏移.
 */
int CTransportStreamParser::Parser(const unsigned char* & pBuf, int iBufLen,
		unsigned short & pid) throw (IException)
{
	assert(pBuf && iBufLen > 0);
	memset(&m_tsPkt, 0, sizeof(TransportPacket));
	int iRet = 0;
	CBits bit;
	bit.Init(const_cast<unsigned char*>(pBuf), iBufLen);
	m_tsPkt.syncByte = bit.Read(8);
	m_tsPkt.transportErrorIndicator = bit.Read() == 1;
	assert(0 == m_tsPkt.transportErrorIndicator);
	if (m_tsPkt.transportErrorIndicator)
	{
		CMpegtsException("TS包兼容错误.", 1, "");
	}

	m_tsPkt.payloadUnitStartIndicator = bit.Read() == 1;
	m_tsPkt.transportPriority = bit.Read() == 1;
	m_tsPkt.pid = bit.Read(13);
	m_tsPkt.transportScramblingControl = bit.Read(2);
	m_tsPkt.adaptationFieldControl = bit.Read(2);
	m_tsPkt.continuityCounter = bit.Read(4);
	iRet += 4;
	if (0x2 == m_tsPkt.adaptationFieldControl
			|| 0x3 == m_tsPkt.adaptationFieldControl)
	{
		TransportPacket::adaptation_field& adpt = m_tsPkt.AdaptationField;
		//* 由于adaptation_field_length的长度不包括它自己.所以,还要加上它自身的长度.
		//* 为了避免所谓的魔数而使用了一个静态,整形,常量.
		static const int size_adaptation_field_length = 1;
		//* 
		adpt.adaptation_field_length = bit.Read(8);
		adpt.discontinuity_indicator = bit.Read() == 1;
		adpt.random_access_indicator = bit.Read() == 1;
		adpt.elementary_stream_priority_indicator = bit.Read() == 1;
		adpt.PCR_flag = bit.Read() == 1;
		adpt.OPCR_flag = bit.Read() == 1;
		adpt.splicing_point_flag = bit.Read() == 1;
		adpt.transport_private_data_flag = bit.Read() == 1;
		adpt.adaptation_field_extension_flag = bit.Read() == 1;

		if (adpt.PCR_flag)
		{
			adpt.program_clock_reference_base = Get33_1_32(bit);
			bit.Read(6);
			adpt.program_clock_reference_extension = bit.Read(9);
		}
		else
		{
			adpt.program_clock_reference_base = 0;
			adpt.program_clock_reference_extension = 0;
		}

		if (adpt.OPCR_flag)
		{
			adpt.original_program_clock_reference_base = Get33_1_32(bit);
			bit.Read(6);
			adpt.original_program_clock_reference_extension = bit.Read(9);

		}
		else
		{
			adpt.original_program_clock_reference_base = 0;
			adpt.original_program_clock_reference_extension = 0;
		}

		if (adpt.splicing_point_flag)
		{
			adpt.splice_countdown = bit.Read(8);
		}
		else
		{
			adpt.splice_countdown = 0;
		}

		if (adpt.transport_private_data_flag)
		{
			adpt.transport_private_data_length = bit.Read(8);
			for (unsigned char i = 0; i < adpt.transport_private_data_length;
					++i)
			{
				//* 私有数据.未处理.
			}
		}
		else
		{
			adpt.transport_private_data_length = 0;
		}

		if (adpt.adaptation_field_extension_flag)
		{
			adpt.adaptation_field_extension_length = bit.Read(8);
			adpt.ltw_flag = bit.Read() == 1;
			adpt.piecewise_rate_flag = bit.Read() == 1;
			adpt.seamless_splice_flag = bit.Read() == 1;
			bit.Read(5);
			if (adpt.ltw_flag)
			{
				adpt.ltw_valid_flag = bit.Read() == 1;
				adpt.ltw_offset = bit.Read(15);
			}
			if (adpt.piecewise_rate_flag)
			{
				bit.Read(2);
				adpt.piecewise_rate = bit.Read(22);
			}
			if (adpt.seamless_splice_flag)
			{
				adpt.splice_type = bit.Read(4);
				__int64 i64 = bit.Read(3);
				i64 <<= 30;
				bit.Read();
				i64 += bit.Read(15);
				i64 <<= 15;
				bit.Read();
				i64 += bit.Read(15);
				bit.Read();
				adpt.DTS_next_AU = static_cast<double>(i64);
			}
		}
		else
		{
			adpt.adaptation_field_extension_length = 0;
			adpt.ltw_flag = false;
			adpt.piecewise_rate_flag = false;
			adpt.seamless_splice_flag = false;
			adpt.ltw_valid_flag = false;
			adpt.ltw_offset = 0;
			adpt.piecewise_rate = 0;
			adpt.splice_type = 0;
			adpt.DTS_next_AU = 0;
		}
		iRet += adpt.adaptation_field_length;
		iRet += size_adaptation_field_length;
	}
	else if (0x1 == m_tsPkt.adaptationFieldControl
			|| 0x3 == m_tsPkt.adaptationFieldControl)
	{
		//* data. do nothing.
	}

	pid = m_tsPkt.pid;

	m_bContinuity = false;
	//* 更新包计数器.
#ifndef __use_array__
	map<unsigned short, unsigned char>::iterator pItr = __FindCounter(
			m_tsPkt.pid, m_tsPkt.continuityCounter);
	int iChecker = m_tsPkt.continuityCounter - pItr->second;
#else
	pid_counter_table* p = __FindCounter(pid, m_tsPkt.continuityCounter);
	int iChecker = m_tsPkt.continuityCounter - p->cnt;
#endif
	if ((iChecker == 1) || (iChecker == -15))
	{
		m_bContinuity = true;
	}
	else
	{
		//* The continuity_counter shall not be incremented 
		//* when the adaptation_field_control of the packet equals '00' or '10'.
		//* (iso/iec 13818-1:2000)
		//* 也就是说在adaptationFieldControl字段为0或者2时,
		//* 包计数器的差为0也认为是连续的.
		if (0x2 == m_tsPkt.adaptationFieldControl
				|| 0x0 == m_tsPkt.adaptationFieldControl)
		{
			if (0 == iChecker)
			{
				m_bContinuity = true;
			}
		}
	}
#ifndef __use_array__
	pItr->second = m_tsPkt.continuityCounter;
#else
	p->cnt = m_tsPkt.continuityCounter;
#endif
	return iRet;
}


#ifndef __use_array__
void CTransportStreamParser::ResetCounter() throw ()
{
	m_LastContinuityCounter.clear();
}

//* 可以保证提供正确的Iteration.
map<unsigned short, unsigned char>::iterator CTransportStreamParser::__FindCounter(
		unsigned short pid, unsigned char cnt)
{
	map<unsigned short, unsigned char>::iterator itr;
	for (itr = m_LastContinuityCounter.begin();
			itr != m_LastContinuityCounter.end(); ++itr)
	{
		if (itr->first == pid)
		{
			return itr;
		}
	}
	//* 将引用计数器变成其上一个值,然后保存起来.
	if (cnt == 0)
	{
		cnt = 15;
	}
	else
	{
		--cnt;
	}
	//* not found,create one
	m_LastContinuityCounter.insert(ContinuityPair(pid, cnt));
	//* recursion call.
	return __FindCounter(pid, cnt);
}
#else
void CTransportStreamParser::ResetCounter() throw ()
{
	delete [] m_counter;
	m_counter = NULL;
	m_counterCnt = 0;
}

pid_counter_table* CTransportStreamParser::__FindCounter(unsigned short pid, unsigned char cnt)
{
	for (unsigned int i = 0; i < m_counterCnt; ++i)
	{
		if (m_counter[i].pid == pid)
		{
			return &m_counter[i];
		}			
	}
	if (cnt == 0)
	{
		cnt = 15;
	}
	else
	{
		--cnt;
	}
	//* 模拟DSHOW的BaseClass.
	pid_counter_table* tmp = new pid_counter_table[m_counterCnt+1];
	memcpy(tmp, m_counter, sizeof(pid_counter_table) * (m_counterCnt)); 
	delete [] m_counter;
	m_counter = tmp;
	m_counter[m_counterCnt].cnt = cnt;
	m_counter[m_counterCnt].pid = pid;
	return &m_counter[m_counterCnt++];
}
#endif
} // namespace wzd
