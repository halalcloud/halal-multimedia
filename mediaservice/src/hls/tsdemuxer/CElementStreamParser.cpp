
#include <cassert>
#include "wzdBits.h"
#include "CElementStreamParser.h"
namespace wzd {
#if 0
long long CElementStreamParser::rotate = 0x1FFFFFFFF;
#endif
CElementStreamParser::CElementStreamParser()
{
	memset(&m_pes, 0, sizeof(PESPacket));
#if 0
	m_uiFactor = 0;
	m_LastDts = 0;
#endif
}

CElementStreamParser::~CElementStreamParser()
{

}

/**
 * 解析字节流.
 * 必须是PES头开始的数据
 * @exception IException 数据错误抛出异常.
 * @param[in] pBuf 数据包指针.
 * @param[in] 数据包大小.
 * @return 返回的是数据区的偏移
 */
int CElementStreamParser::Parser(const unsigned char* & pBuf, int iBufLen) throw(IException) 
{
	assert(pBuf && iBufLen > 0);
	if (false == check(pBuf, iBufLen))
	{
		return 0;
	}
	memset(&m_pes, 0, sizeof(PESPacket));
	CBits bit;
	int iRet = 0;
	bit.Init(const_cast<unsigned char*>(pBuf), iBufLen);
	m_pes.packet_start_code_prefix[0] = bit.Read(8);
	m_pes.packet_start_code_prefix[1] = bit.Read(8);
	m_pes.packet_start_code_prefix[2] = bit.Read(8);

	if (m_pes.packet_start_code_prefix[0] != 0 &&
		m_pes.packet_start_code_prefix[1] != 0 &&
		m_pes.packet_start_code_prefix[2] != 1)
	{
		throw CMpegtsException("1", 1, "");
	}
	assert(m_pes.packet_start_code_prefix[0] == 0 &&
		m_pes.packet_start_code_prefix[1] == 0 &&
		m_pes.packet_start_code_prefix[2] == 1);
	
	m_pes.stream_id			= bit.Read(8);
	m_pes.PES_packet_length = bit.Read(16);

	iRet += 6;
	static const int program_stream_map		= 0xbc;
	static const int padding_stream			= 0xbe;
	static const int padding_stream_1		= 0xbd;
	static const int private_stream_2		= 0xbf;
	static const int ISO_13818_3_7__11172_3__14496_3_audio_stream_number_xxxxx		= 0xc0;
	static const int ISO_13818_2__H262__11172_2__14496_2_vidio_stream_number_xxxx	= 0xe0;
	static const int ECM	= 0xf0;
	static const int EMM	= 0xf1;
	static const int DSMCC_stream				= 0xf2;
	static const int ISO_13522_stream			= 0xf3;
	static const int ITU_Rec_H222_TypeA_stream = 0xf4;
	static const int ITU_Rec_H222_TypeB_stream = 0xf5;
	static const int ITU_Rec_H222_TypeC_stream = 0xf6;
	static const int ITU_Rec_H222_TypeD_stream = 0xf7;
	static const int ITU_Rec_H222_TypeE_stream = 0xf8;
	static const int Alcillary_stream = 0xf9;
	static const int ISO_14496_SL_packetized_stream = 0xfa;
	static const int ISO_14496_FlexMux_stream		= 0xfb;
	static const int program_stream_directory = 0xff;
	if (m_pes.stream_id != program_stream_map &&
		m_pes.stream_id != padding_stream &&
		m_pes.stream_id != private_stream_2 &&
		m_pes.stream_id != EMM &&
		m_pes.stream_id != ECM &&
		m_pes.stream_id != program_stream_directory &&
		m_pes.stream_id != DSMCC_stream &&
		m_pes.stream_id != ITU_Rec_H222_TypeE_stream)
	{
		int iHeaderLen = 0;
		if (0x2 != bit.Read(2))
		{
			assert(0);
			throw CMpegtsException("2", 1, "");

		}
		m_pes.PES_scrambling_control	= bit.Read(2);
		m_pes.PES_priority				= bit.Read() == 1;
		m_pes.data_alignment_indicator	= bit.Read() == 1;
		m_pes.copyright					= bit.Read() == 1;
		m_pes.original_or_copy			= bit.Read() == 1;
		m_pes.PTS_DTS_flags				= bit.Read(2);
		m_pes.ESCR_flag					= bit.Read() == 1;
		m_pes.ES_rate_flag				= bit.Read() == 1;
		m_pes.DSM_trick_mode_flag		= bit.Read() == 1;
		m_pes.additional_copy_info_flag = bit.Read() == 1;
		m_pes.PES_CRC_flag				= bit.Read() == 1;
		m_pes.PES_extension_flag		= bit.Read() == 1;
		m_pes.PES_header_data_length	= bit.Read(8);
		iRet += 3;
		iRet += m_pes.PES_header_data_length;
		if (m_pes.PTS_DTS_flags == 0x2)
		{
			if (0x2 != bit.Read(4))
			{
				assert(0);
				throw CMpegtsException("3", 1, "");
			}
			m_pes.PTS = Get33_3_15_15(bit);
			iHeaderLen += 5;
			//* if no pts.
			m_pes.DTS = m_pes.PTS;
		}
		else if (m_pes.PTS_DTS_flags == 0x3)
		{
			if (0x3 != bit.Read(4))
			{
				assert(0);
				throw CMpegtsException("4", 1, "");
			}
			m_pes.PTS = Get33_3_15_15(bit);
			if (0x1 != bit.Read(4))
			{
				assert(0);
				throw CMpegtsException("5", 1, "");
			}
			m_pes.DTS = Get33_3_15_15(bit);
			iHeaderLen += 10;
		}
		else if (0 == m_pes.PTS_DTS_flags)
		{
			m_pes.PTS = m_pes.DTS = -1;
		}		
		else
		{
			assert(0);
			throw CMpegtsException("6", 1, "非法的PTS_DTS_Flags");
		}
		
		if (m_pes.ESCR_flag)
		{
			bit.Read(2);
			m_pes.ESCR_base = Get33_3_15_15(bit);
			m_pes.ESCR_extension = bit.Read(9);
			bit.Read();
			iHeaderLen += 6;
		}
		if (m_pes.ES_rate_flag)
		{
			bit.Read();
			m_pes.ES_rate = bit.Read(22);
			bit.Read();
			iHeaderLen += 3;
		}
		if (m_pes.DSM_trick_mode_flag)
		{
			static const int fast_forward	= 0x0;
			static const int slow_motion	= 0x1;
			static const int freeze_frame	= 0x2;
			static const int fast_reverse	= 0x3;
			static const int slow_reverse	= 0x4;
			m_pes.trick_mode_control = bit.Read(3);
			if (m_pes.trick_mode_control == fast_forward)
			{
				m_pes.field_id = bit.Read(2);
				m_pes.intra_slice_refresh = 1 == bit.Read();
				m_pes.frequency_truncation = bit.Read(2);
			}
			else if (m_pes.trick_mode_control == slow_motion)
			{
				m_pes.rep_cntrl = bit.Read(5);
			}
			else if (m_pes.trick_mode_control == freeze_frame)
			{
				m_pes.field_id = bit.Read(2);
				bit.Read(3);
			}
			else if (m_pes.trick_mode_control == fast_reverse)
			{
				m_pes.field_id = bit.Read(2);
				m_pes.intra_slice_refresh = 1 == bit.Read();
				m_pes.frequency_truncation = bit.Read(2);

			}
			else if (m_pes.trick_mode_control == slow_reverse)
			{
				m_pes.rep_cntrl = bit.Read(5);
			}
			iHeaderLen += 1;
		}
		if (m_pes.additional_copy_info_flag)
		{
			bit.Read();
			m_pes.additional_copy_info = bit.Read(7);
			iHeaderLen += 1;
		}
		if (m_pes.PES_CRC_flag)
		{
			m_pes.previous_PES_packet_CRC = bit.Read(16);
			iHeaderLen += 2;
		}
		if (m_pes.PES_extension_flag)
		{
			m_pes.PES_private_data_flag					= 1 == bit.Read();
			m_pes.pack_header_field_flag				= 1 == bit.Read();
			m_pes.program_packet_sequence_counter_flag	= 1 == bit.Read();
			m_pes.P_STD_buffer_flag						= 1 == bit.Read();
			bit.Read(3);
			m_pes.PES_extension_flag_2					= 1 == bit.Read();
			iHeaderLen += 1;
			if (m_pes.PES_private_data_flag)
			{
				for (int i = 0; i < 16; ++i)
				{
					m_pes.PES_private_data[i] = bit.Read(8);
				}
				iHeaderLen += 16;
			}
			if (m_pes.pack_header_field_flag)
			{
				m_pes.pack_field_length = bit.Read(8);
				assert(0);
				//* ISO_IEC 11172-1
			}
			if (m_pes.program_packet_sequence_counter_flag)
			{
				bit.Read();
				m_pes.program_packet_sequence_counter = bit.Read(7);
				bit.Read();
				m_pes.MPEG1_MPEG2_identifier = 1 == bit.Read();
				m_pes.original_stuff_length = bit.Read(6);
				iHeaderLen += 2;
			}
			if (m_pes.P_STD_buffer_flag)
			{
				if (0x1 != bit.Read(2))
				{
					assert(0);
					throw CMpegtsException("", 1, "");
				}
				m_pes.P_STD_buffer_scale = 1 == bit.Read();
				m_pes.P_STD_buffer_size = bit.Read(13);
				iHeaderLen += 2;
			}
			if (m_pes.PES_extension_flag_2)
			{
				bit.Read();
				m_pes.PES_extension_field_length = bit.Read(7);
				for (int i = 0; i < m_pes.PES_extension_field_length; ++ i)
				{
					//* do nothing.
				}
				iHeaderLen += m_pes.PES_extension_field_length;
				iHeaderLen += 1;
			}
		}
		//* struffing_byte
		//m_pes.PES_header_data_length - iHeaderLen;
		for (int i = 0; i < 1; ++i)
		{
			//* don nothing
		}
		for (int i = 0; i < 1; ++i)
		{
			//* don nothing
		}
	}
	else if (m_pes.stream_id == program_stream_map ||
		m_pes.stream_id == private_stream_2 ||
		m_pes.stream_id == ECM ||
		m_pes.stream_id == EMM ||
		m_pes.stream_id == program_stream_directory ||
		m_pes.stream_id == DSMCC_stream ||
		m_pes.stream_id == ITU_Rec_H222_TypeE_stream)
	{
		//* parser psm.
		switch (m_pes.stream_id)
		{
		case program_stream_map:
			{
				assert(m_pes.PES_packet_length < 0x3FA);
				__ProgramStreamMap(bit);
			}
			break;
		case private_stream_2:
			{
				__PrivateStream2(bit);
			}
			break;
		case program_stream_directory:
			{

			}
			break;
		default:
			break;
		}
		//iRet += m_pes.PES_packet_length;
	}
	else if (m_pes.stream_id == padding_stream)
	{

		//* do nothing
	}

#if 0 //* 不再计算时间
	//* calc pts,dts
	if (m_pes.DTS - m_LastDts > m_pes.DTS)
	{
		++m_uiFactor;
	}
	m_pes.PTS += m_uiFactor * rotate;
	m_pes.DTS += m_uiFactor * rotate;
	m_LastDts = m_pes.DTS;
#endif
	return iRet;
}

/**
 * 检测数据的合法性.
 * 前三个字节必须是0x000001
 */
bool CElementStreamParser::check(const unsigned char* & pBuf, int iBufLen) throw() 
{
	if (iBufLen > 3 && pBuf[0] == 0x0 && pBuf[1] == 0x0 && pBuf[2] == 0x1)
	{
		return true;
	}
	return false;
}

void CElementStreamParser::__ProgramStreamMap(const CBits& bit)
{
	//* 暂未实现.
	assert(0);
}

void CElementStreamParser::__ProgramStreamDirectory(const CBits& bit)
{
	//* 暂未实现.
	assert(0);
}

void CElementStreamParser::__PrivateStream2(const CBits& bit)
{
	//* 暂未实现.
	assert(0);
}
} // namespace wzd
