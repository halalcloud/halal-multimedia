/**
 *	@file Bits.h
 *	�����װ�� VideoLAN ��֯��Laurent Aimar <fenrir@via.ecp.fr>
 *	��д��ͷ�ļ�bits.h : Bit handling helpers.
 */
#ifndef WZD_BITS_H
#define WZD_BITS_H
#include <bitset>
#include "wzdexception.h"

namespace wzd
{

/**
 *	λ��������.
 *	@author ��ռ��
 *	@data 2012-04-13
 *	@version 1.0.1
 *	@warning .
 */
class CBits
{
public:
	/**
	 *	���ô˹��캯����,�������Init����.
	 */
	CBits(void);
	/**
	 *	
	 */
	explicit CBits(CBits& bits);
	CBits(unsigned char* pucData, unsigned int uiLen);
	~CBits(void);

	/**
	 *	��ȡָ��Ŀ�ı���.
	 *	@param[in]	iCount Ҫ��ȡ�ı�����.
	 *	@note ����������Ϊ:0x01020304,
	 *			��:read(32)�õ�������Ϊ:0x01020304.
	 *	@return ��������ֵ.
	 *	@warning .
	 *	@exception ��ȡʧ���׳��쳣
	 */
    int Read(int iCount = 1) /*throw (IException)*/;

	/**
	 *	����ָ��λ�õı���.
	 *	@ʵ����δ���.
	 *	@param[in] iCount Ҫд��ı�����.
	 *	@param[in]	Ҫд���ֵ.
	 *	@return NULL.
	 *	@exception �����׳��쳣.
	 */
    void Write(int iCount, int iVal) /*throw (IException)*/;

	/**
	 *	����ָ�����ȵı���.
	 *	@param[in] iLen Ҫ�����ı�����.�����Ǹ���.
	 *	@return NULL.
	 *	@note ���ﲻ����ȷ���ж�.���Խ����Read,Write������
	 *			�����ж�.
	 */
	void Skip(int iLen);

	/**
	 *	��ʼ������.
	 *	@param[in]	pucData ָ��������.
	 *	@param[in]	uiLen	���������ֽ���.
	 *	@return 
	 *	@remark	�����ظ���ʼ��.�����ܲ���ʼ��.
	 */
    void Init(unsigned char* pucData, unsigned int uiLen) /*throw (IException)*/;
	/**
	 *	�ƶ�����һ�ֽڿ�ʼλ��.
	 *	���պ����ֽڿ�ʼ�����ƶ�.
	 *	@note
	 *		��������ƶ������ȷ��.
	 */
	void Align(void);
	/**
	 *	�����һλ��ֵ.
	 *	@return ��1����1,��0����0.
	 *	@note �ٶ����˵�.
	 */
	int Test(void);

	// ��ȡ��ǰλ�������������е�����.
	int GetCurPos(void);

	/**
	 *	�ж��Ƿ񵽴�������н�β.
	 *	@return ������󷵻�ture,��֮����false.
	 */
	bool IsEnd(void);
	/**
	 * �޷�������ָ�����ײ������
	 */
	unsigned int get_ue_golomb() throw(IException);
	/**
	 * �з�������ָ�����ײ������
	 */
	int get_se_golomb() throw(IException);
protected:
	bool				m_bInit;	//* �Ƿ��ʼ����־.�����κκ���ǰ���뱣֤��Ϊtrue.
	unsigned char*		m_pStart;	//* �������е���ʼλ��.
	unsigned char*		m_pCur;		//* �������еĵ�ǰλ��.
	unsigned char*		m_pEnd;		//* �������еĽ���λ��.
	int					m_iLeft;	//* �ɷ��ʵı������е�λ������.
	static unsigned int m_uiMask[33];	
	// ��ʼ���ڲ���Ա
	void __Init(void);
};
}
#endif
