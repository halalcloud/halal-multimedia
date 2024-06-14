#include "wzdBits.h"
#include <cassert>
namespace wzd {
//* ���ڽ�����Ҫ��λ��0.
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
// ��ȡָ��Ŀ�ı���
int CBits::Read(int iCount/* = 1*/)
{
	//* һ�ζ�ȡ���ܳ���32.
	if (iCount > 32)
	{
		return 0;
	}

	int				i_shr		= 0;
	unsigned int	uiResult	= 0;//* ������Int����UINT�д��۲�

	while( iCount > 0 )
	{
		//* �����ֽ����н�β,�˳�ѭ��.
		if( m_pCur >= m_pEnd )
		{
			uiResult = 0;
			break;
		}

		//* ��ȡ�ĳ���С��8(С��1�ֽ�).
		if( ( i_shr = m_iLeft - iCount ) >= 0 )
		{
			//*����: 0101010101,Ҫȡ�������λ,���뽫������6λ���
			//*	xxxxxx01,Ȼ�����6λ��0.����������00000011.
			uiResult |= ( *m_pCur >> i_shr )&m_uiMask[iCount];
			m_iLeft -= iCount;

			//* 1�ֽڶ���.ָ���ƶ�,���ָ�����ֵ.
			if( m_iLeft == 0 )
			{
				++m_pCur;
				m_iLeft = 8;
			}
			//* ���ؽ��.
			return uiResult;
		}
		else//* ��ȡ�ĳ��ȴ���8(����1�ֽ�)
		{
			//*	��: 01010101010101010101010101010101,ȡ����12λ.
			//* 1. ��8λȡ��
			//*	2. ��8λ����,Ϊ������4λ���ط�.
			//* 3. ����8λ��,��С��8λ���߼�����
			uiResult |= (*m_pCur&m_uiMask[m_iLeft]) << -i_shr;
			iCount  -= m_iLeft;
			++m_pCur;
			m_iLeft = 8;
		}
	}

	return uiResult;
}

// ����ָ��λ�õı���
void CBits::Write(int iCount, int iVal)
{
	//* ��û�п�����.
	while( iCount > 0 )
	{
		if( m_pCur >= m_pEnd )
		{
			break;
		}

		--iCount;

		//* ��λ�����λ�������λ,
		//* ��0x01�����λ����0.
		if( ( iVal >> iCount )&0x01 )//* д��λΪ1.
		{
			//* ��1����m_iLeft-1λ����Ҫд���λ.
			//* ��ԭֵ���ֻ��д��λ��1,����λ����.
			*m_pCur |= 1 << ( m_iLeft - 1 );
		}
		else//* д��λΪ0.
		{
			//* ��1����m_iLeft-1λ����Ҫд���λ.
			//* ȡ����Ҫд���λ���0,����λ���1.
			//* ��ԭֵ����ֻ��д��λ��0,����λ����.
			*m_pCur &= ~( 1 << ( m_iLeft - 1 ) );
		}
		--m_iLeft;

		if( 0 == m_iLeft)//* ����8�����ƶ�ָ��.
		{
			++m_pCur;
			m_iLeft = 8;
		}
	}
}


// ����ָ�����ȵı���
void CBits::Skip(int iLen)
{
	m_iLeft -= iLen;

	//* ������
	if (m_iLeft <= 0)
	{
		//* �����8�ļ���
		//* ���ڱ���ȥ��8,���Լӻ���.
		int iBytes = (-m_iLeft +8) /8;

		//* �ƶ�ָ�� 
		m_pCur += iBytes;

		//* �������ӻ���
		m_iLeft += 8 * iBytes;
	}
}

// ��ʼ������.
void CBits::Init(unsigned char* pucData, unsigned int uiLen)
{
	assert(pucData && uiLen);

	m_pStart	= pucData;
	m_pEnd		= pucData + uiLen;
	m_pCur		= m_pStart;
	m_iLeft		= 8;//* 1���ֽڵı�����

	m_bInit		= true;
}

// �ƶ�����һ�ֽڿ�ʼλ��.
void CBits::Align(void)
{
	if (m_iLeft != 8)
	{
		m_iLeft = 8;
		++m_pCur;
	}
}

// ��⵱ǰλ��ֵ
int CBits::Test(void)
{
	return CBits(*this).Read(1);
}

// ��ʼ���ڲ���Ա
void CBits::__Init(void)
{
	m_bInit		= false;
	m_pStart	= NULL;
	m_pEnd		= NULL;
	m_pCur		= NULL;
	m_iLeft		= 0;
}
// ��ȡ��ǰλ�������������е�����.
int CBits::GetCurPos(void)
{
    return  8 * ( m_pCur - m_pStart ) + 8 - m_iLeft;
}

// �ж��Ƿ񵽴�������н�β
bool CBits::IsEnd(void)
{
	return m_pCur >= m_pEnd ? 1 : 0;
}
}