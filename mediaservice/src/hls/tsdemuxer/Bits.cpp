#include "wzdBits.h"
#include <cassert>
namespace wzd {
//* 用于将不需要的位清0.
unsigned int CBits::m_uiMask[33] =
{	0x00,
	0x01,      0x03,      0x07,      0x0f,
	0x1f,      0x3f,      0x7f,      0xff,
	0x1ff,     0x3ff,     0x7ff,     0xfff,
	0x1fff,    0x3fff,    0x7fff,    0xffff,
	0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
	0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
	0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
	0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
};

CBits::CBits(void)
{
	__Init();
}

CBits::~CBits(void)
{
	__Init();
}

CBits::CBits(CBits& bits)
{
	this->m_bInit	= bits.m_bInit;
	this->m_iLeft	= bits.m_iLeft;
	this->m_pCur	= bits.m_pCur;
	this->m_pStart	= bits.m_pStart;
	this->m_pEnd	= bits.m_pEnd;
}

CBits::CBits(unsigned char* pucData, unsigned int uiLen)
{
	__Init();
	Init(pucData, uiLen);
}
// 读取指数目的比特
int CBits::Read(int iCount/* = 1*/)
{
	//* 一次读取不能超过32.
	if (iCount > 32)
	{
		return 0;
	}

	int				i_shr		= 0;
	unsigned int	uiResult	= 0;//* 这里是Int还是UINT有待观察

	while( iCount > 0 )
	{
		//* 超出字节序列结尾,退出循环.
		if( m_pCur >= m_pEnd )
		{
			uiResult = 0;
			break;
		}

		//* 读取的长度小于8(小于1字节).
		if( ( i_shr = m_iLeft - iCount ) >= 0 )
		{
			//*　如: 0101010101,要取其左侧两位,必须将其右移6位变成
			//*	xxxxxx01,然后将左侧6位清0.方法是与上00000011.
			uiResult |= ( *m_pCur >> i_shr )&m_uiMask[iCount];
			m_iLeft -= iCount;

			//* 1字节读完.指针移动,并恢复索引值.
			if( m_iLeft == 0 )
			{
				++m_pCur;
				m_iLeft = 8;
			}
			//* 返回结果.
			return uiResult;
		}
		else//* 读取的长度大于8(大于1字节)
		{
			//*	如: 01010101010101010101010101010101,取其左12位.
			//* 1. 将8位取出
			//*	2. 将8位左移,为将来的4位留地方.
			//* 3. 不足8位的,按小于8位的逻辑处理
			uiResult |= (*m_pCur&m_uiMask[m_iLeft]) << -i_shr;
			iCount  -= m_iLeft;
			++m_pCur;
			m_iLeft = 8;
		}
	}

	return uiResult;
}

// 设置指定位置的比特
void CBits::Write(int iCount, int iVal)
{
	//* 还没有看明白.
	while( iCount > 0 )
	{
		if( m_pCur >= m_pEnd )
		{
			break;
		}

		--iCount;

		//* 移位后最高位到达最低位,
		//* 与0x01相与高位被清0.
		if( ( iVal >> iCount )&0x01 )//* 写入位为1.
		{
			//* 将1左移m_iLeft-1位到达要写入的位.
			//* 与原值相或只有写入位置1,其它位不变.
			*m_pCur |= 1 << ( m_iLeft - 1 );
		}
		else//* 写入位为0.
		{
			//* 将1左移m_iLeft-1位到达要写入的位.
			//* 取反后要写入的位变成0,其它位变成1.
			//* 与原值相与只有写入位置0,其它位不变.
			*m_pCur &= ~( 1 << ( m_iLeft - 1 ) );
		}
		--m_iLeft;

		if( 0 == m_iLeft)//* 超过8比特移动指针.
		{
			++m_pCur;
			m_iLeft = 8;
		}
	}
}


// 跳过指定长度的比特
void CBits::Skip(int iLen)
{
	m_iLeft -= iLen;

	//* 不够减
	if (m_iLeft <= 0)
	{
		//* 检测是8的几倍
		//* 由于被减去了8,所以加回来.
		int iBytes = (-m_iLeft +8) /8;

		//* 移动指针 
		m_pCur += iBytes;

		//* 将索引加回来
		m_iLeft += 8 * iBytes;
	}
}

// 初始化对象.
void CBits::Init(unsigned char* pucData, unsigned int uiLen)
{
	assert(pucData && uiLen);

	m_pStart	= pucData;
	m_pEnd		= pucData + uiLen;
	m_pCur		= m_pStart;
	m_iLeft		= 8;//* 1个字节的比特数

	m_bInit		= true;
}

// 移动到下一字节开始位置.
void CBits::Align(void)
{
	if (m_iLeft != 8)
	{
		m_iLeft = 8;
		++m_pCur;
	}
}

// 检测当前位的值
int CBits::Test(void)
{
	return CBits(*this).Read(1);
}

// 初始化内部成员
void CBits::__Init(void)
{
	m_bInit		= false;
	m_pStart	= NULL;
	m_pEnd		= NULL;
	m_pCur		= NULL;
	m_iLeft		= 0;
}
// 获取当前位在整个比特流中的索引.
int CBits::GetCurPos(void)
{
    return  8 * ( m_pCur - m_pStart ) + 8 - m_iLeft;
}

// 判断是否到达比特序列结尾
bool CBits::IsEnd(void)
{
	return m_pCur >= m_pEnd ? 1 : 0;
}
}