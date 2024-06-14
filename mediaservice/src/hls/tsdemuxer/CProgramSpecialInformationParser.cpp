
#include <cassert>
#include "CProgramSpecialInformationParser.h"
#include "mpeg2.h"
#include "CTSDeMuxer.h"
#include "wzdBits.h"
using namespace std;
namespace wzd {
static const unsigned short  CRC32LEN	= 4;
CProgramSpecialInformationParser::CProgramSpecialInformationParser() throw() 
{
	m_bPatFinished = false;

}

CProgramSpecialInformationParser::~CProgramSpecialInformationParser() throw() 
{

}

/**
 * 解析PSI字节流.
 * 一次必须是一个TS包的负载.
 * 因为PMT和PAT全部保存在类中,所以根据PID可知道是什么数据
 * @exception IException
 * @param[in] pBuf 数据包指针.
 * @param[in] 数据包大小.
 * @param[in] pid 数据包的PID值.
 * @return 解析成功返回0,解析中返回1.
 * @note 当Pat和Pmt都解析完成时才返回0.
 */
int CProgramSpecialInformationParser::Parser(const unsigned char* & pBuf, int iBufLen, unsigned short pid) throw(IException) 
{
	assert(pBuf && iBufLen > 0);

	if (pid == 0x00)	//* pat info
	{
		__PATParse(const_cast<unsigned char*>(pBuf), iBufLen);
	}
	else //* non-pat info
	{
		if (__IsPMT(pid))
		{
			__PMTParse(const_cast<unsigned char*>(pBuf), iBufLen);
		}
		else
		{
		}
	}

	bool bRet = this->IsPATFinshed() &&
		(__GetProgramCount() == m_pmt.size());
	return !bRet;
}

/**
 * 获取Pat表.
 * 外部不能修改其内容.
 * 在执行下次解析之前内容可用.执行过解析之后内容会变化.
 */
bool CProgramSpecialInformationParser::GetPAT(vector<ProgramAssociationSection> & pat) throw() 
{
	pat = m_pat;
	return true;
}


bool CProgramSpecialInformationParser::IsPSI(unsigned short pid)
{
	vector<ProgramAssociationSection>::const_iterator itr;

	for (itr = m_pat.begin(); itr != m_pat.end(); ++itr)
	{
		vector<ProgramAssociationSection::pat_info>::const_iterator itr1;
		for(itr1 = itr->ProgramMapPids.begin();
			itr1 != itr->ProgramMapPids.end(); ++itr1)
		{
			if (itr1->program_number != 0 &&
				itr1->network_or_program_map_PID == pid)
			{
				return true;
			}
		}
	}
	return false;
}
/**
 * 获取Pmt表.
 * 外部不能修改其内容.
 * 在执行下次解析之前内容可用.执行过解析之后内容会变化.
 */
const vector<ProgramMapSection>& CProgramSpecialInformationParser::GetPMT() const throw() 
{
  return m_pmt;

}

/**
 * 检测数据的合法性.
 */
bool CProgramSpecialInformationParser::check() throw() 
{
  return 0l;

}

int CProgramSpecialInformationParser::__PATParse(unsigned char* pBuf, int iBufLen) throw(IException) 
{
	m_bPatFinished = false;
	CBits bits;
	bits.Init(pBuf, iBufLen);
	static const unsigned short	 OFFSET		= 5;
	ProgramAssociationSection pat;
	//* table id
	pat.table_id					= bits.Read(8);
	//* section_syntax_indicator
	pat.section_syntax_indicator	= bits.Read();
	//* '0'
	bool bConstBit					= bits.Read();
	assert(bConstBit == 0);
	if (bConstBit != 0)
	{
		throw CMpegtsException("",1,"");
	}

	//* reserved
	bits.Read(2);
	//* section_length
	pat.section_length				= bits.Read(12);
	//* transport_stream_id
	pat.transport_stream_id			= bits.Read(16);
	//* reserved 
	bits.Read(2);
	//* version_number
	pat.version_number				= bits.Read(5);
	//* current_next_indicator
	pat.current_next_indicator		= bits.Read();
	//* section_number
	pat.section_number				= bits.Read(8);
	//* last_section_number
	pat.last_section_number			= bits.Read(8);
	assert(pat.section_number <= pat.last_section_number);
	if (pat.section_number > pat.last_section_number)
	{
		throw CMpegtsException("", 1, "");
	}
	

	unsigned short uiLoop = (pat.section_length - CRC32LEN- OFFSET)/4;
	for (unsigned short ui = 0; ui < uiLoop; ++ ui)
	{
		ProgramAssociationSection::pat_info info;
		info.program_number = bits.Read(16);
		bits.Read(3);
		info.network_or_program_map_PID = bits.Read(13);
		if (info.program_number == 0)
		{
			pat.NetworkPids.push_back(info);
		}
		else
		{
			pat.ProgramMapPids.push_back(info);
		}			
	}

	if (false == __IsPatSectionExist(pat.section_number))
	{
		m_pat.push_back(pat);
	}

	//* 表明PAT是否分析完.
	m_bPatFinished = pat.last_section_number == pat.section_number;
	return 0l;
}

int CProgramSpecialInformationParser::__PMTParse(unsigned char* pBuf, int iBufLen) throw(IException) 
{
	static unsigned short const OFFSET = 12;
	
	CBits bits;
	bits.Init(pBuf, iBufLen);
	ProgramMapSection pmt;
	//* table id
	pmt.table_id					= bits.Read(8);
	//* section_syntax_indicator
	pmt.section_syntax_indicator	= bits.Read();
	//* '0'
	bool bConstBit					= bits.Read();
	assert(bConstBit == 0);
	if (bConstBit != 0)
	{
		throw CMpegtsException("",1,"");
	}
	//* reserved
	bits.Read(2);
	//* section_length
	pmt.section_length				= bits.Read(12);
	//* program_number
	pmt.program_number				= bits.Read(16);
	//* reserved 
	bits.Read(2);
	pmt.version_number				= bits.Read(5);
	pmt.current_next_indicator		= bits.Read();
	pmt.section_number				= bits.Read(8);
	pmt.last_section_number			= bits.Read(8);
	//* reserved 
	bits.Read(3);
	pmt.PCR_PID						= bits.Read(13);
	bits.Read(4);
	pmt.program_info_length			= bits.Read(12);
	unsigned short usLoop = pmt.section_length 
		- OFFSET - CRC32LEN - pmt.program_info_length;
	for (unsigned us = 0; us < pmt.program_info_length;)
	{
		//* descriptor
		//* 未处理.
		++us;
		bits.Read(8);
	}
	
	for (unsigned short us = 0; us < usLoop; )
	{
		ProgramMapSection::ES_info info;
		info.stream_type = bits.Read(8);
		bits.Read(3);
		info.elementary_PID = bits.Read(13);
		bits.Read(4);
		info.ES_info_length = bits.Read(12);
		for(unsigned short usInner = 0; usInner < info.ES_info_length;)
		{
			//* descriptor
			//* 未处理.
			++usInner;			
			bits.Read(8);
		}
		pmt.esInfo.push_back(info);
		us += 5;
		us += info.ES_info_length;
	}

	if (false == __IsPmtSectionExist(pmt.program_number))
	{
		m_pmt.push_back(pmt);
	}
	return 0l;

}

int CProgramSpecialInformationParser::__NITParse() throw() 
{
  return 0l;

}

int CProgramSpecialInformationParser::__CATParse() throw() 
{
  return 0l;

}

int CProgramSpecialInformationParser::__TSDTParse() throw() 
{
  return 0l;

}

bool CProgramSpecialInformationParser::__IsPMT(unsigned short pid)
{
	std::vector<wzd::ProgramAssociationSection>::const_iterator itr;
	for (itr = m_pat.begin(); itr != m_pat.end(); ++itr)
	{
		std::vector<wzd::ProgramAssociationSection::pat_info>::const_iterator itrPid;
		for (itrPid = itr->ProgramMapPids.begin();
			itrPid != itr->ProgramMapPids.end(); ++ itrPid)
		{
			if (itrPid->network_or_program_map_PID == pid)
			{
				return true;
			}
		}
	}
	return false;
}

bool CProgramSpecialInformationParser::__IsPatSectionExist(unsigned char section_number)
{
	vector<ProgramAssociationSection>::const_iterator itr;
	for (itr = m_pat.begin(); itr != m_pat.end(); ++itr)
	{
		if (itr->section_number == section_number)
		{
			return true;
		}
	}
	return false;
}

bool CProgramSpecialInformationParser::__IsPmtSectionExist(unsigned short program_number)
{
	vector<ProgramMapSection>::const_iterator itr;
	for (itr = m_pmt.begin(); itr != m_pmt.end(); ++itr)
	{
		if (itr->program_number == program_number)
		{
			return true;
		}
	}
	return false;
}

unsigned int CProgramSpecialInformationParser::__GetProgramCount()
{
	unsigned int uiRet = 0;
	std::vector<ProgramAssociationSection>::const_iterator itr;
	for (itr = m_pat.begin(); itr != m_pat.end(); ++itr)
	{
		std::vector<wzd::ProgramAssociationSection::pat_info>::const_iterator itrPid;
		for (itrPid = itr->ProgramMapPids.begin();
			itrPid != itr->ProgramMapPids.end(); ++ itrPid)
		{
			++uiRet;
		}
	}
	return uiRet;
}
} // namespace wzd
