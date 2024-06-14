/////////////////////////////////////////////////////////////////////////////////////////////
// Project:TsFormat
// Author:
// Date:星期一, 三月 5, 2012
// Description:The Wrapper of Bits Stream Writer
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BITWRITE_H__
#define __BITWRITE_H__

#include <string.h>
class CBitWriter                                                                                                                                  
{                                                                                                                                                    
public:                                                                                                                                              
	CBitWriter(unsigned long size) : m_DataSize(size), m_BitCount(0) {                                                                                 
		if (size) {                                                                                                                                  
			m_Data = new unsigned char[size];                                                                                                        
			memset(m_Data, 0, size);                                                                                                          
		} else {                                                                                                                                     
			m_Data = NULL;                                                                                                                           
		}                                                                                                                                            
	}                                                                                                                                                
    ~CBitWriter() { delete [] m_Data; }

	void Write(unsigned int bits, unsigned int bit_count);                                                                                               

	unsigned int GetBitCount()     { return m_BitCount; }                                                                                            
	const unsigned char* GetData() { return m_Data;     }                                                                                            

private:                                                                                                                                            
	unsigned char* m_Data;                                                                                                                          
	unsigned int   m_DataSize;                                                                                                                      
	unsigned int   m_BitCount;                                                                                                                      
};   
#endif // __BITWRITE_H__
