/////////////////////////////////////////////////////////////////////////////////////////////
// Project:TsFormat
// Author:
// Date:星期一, 三月 5, 2012
// Description:The Wrapper of Bits Stream Writer
/////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "BitWriter.h"

void                                                                                                                                                
CBitWriter::Write(unsigned int bits, unsigned int bit_count)                                                                                         
{                                                                                                                                                   
	unsigned char* data = m_Data;                                                                                                                   
	if (m_BitCount+bit_count > m_DataSize*8) return;                                                                                                
	data += m_BitCount/8;                                                                                                                           
	unsigned int space = 8-(m_BitCount%8);                                                                                                          
	while (bit_count) {                                                                                                                             
		unsigned int mask = bit_count==32 ? 0xFFFFFFFF : ((1<<bit_count)-1);                                                                        
		if (bit_count <= space) {                                                                                                                   
			*data |= ((bits&mask) << (space-bit_count));                                                                                            
			m_BitCount += bit_count;                                                                                                                
			return;                                                                                                                                 
		} else {                                                                                                                                    
			*data |= ((bits&mask) >> (bit_count-space));                                                                                            
			++data;                                                                                                                                 
			m_BitCount += space;                                                                                                                    
			bit_count  -= space;                                                                                                                    
			space       = 8;                                                                                                                        
		}                                                                                                                                           
	}                                                                                                                                               
}  
