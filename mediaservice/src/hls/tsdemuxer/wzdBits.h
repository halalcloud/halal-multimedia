/**
 *	@file Bits.h
 *	本类封装了 VideoLAN 组织的Laurent Aimar <fenrir@via.ecp.fr>
 *	所写的头文件bits.h : Bit handling helpers.
 */
#ifndef WZD_BITS_H
#define WZD_BITS_H
#include <bitset>
#include "wzdexception.h"

namespace wzd
{

/**
 *	位操作的类.
 *	@author 王占舵
 *	@data 2012-04-13
 *	@version 1.0.1
 *	@warning .
 */
class CBits
{
public:
	/**
	 *	调用此构造函数后,必须调用Init函数.
	 */
	CBits(void);
	/**
	 *	
	 */
	explicit CBits(CBits& bits);
	CBits(unsigned char* pucData, unsigned int uiLen);
	~CBits(void);

	/**
	 *	读取指数目的比特.
	 *	@param[in]	iCount 要读取的比特数.
	 *	@note 若比特序列为:0x01020304,
	 *			则:read(32)得到的整数为:0x01020304.
	 *	@return 返回数据值.
	 *	@warning .
	 *	@exception 读取失败抛出异常
	 */
    int Read(int iCount = 1) /*throw (IException)*/;

	/**
	 *	设置指定位置的比特.
	 *	@实现暂未理解.
	 *	@param[in] iCount 要写入的比特数.
	 *	@param[in]	要写入的值.
	 *	@return NULL.
	 *	@exception 出错抛出异常.
	 */
    void Write(int iCount, int iVal) /*throw (IException)*/;

	/**
	 *	跳过指定长度的比特.
	 *	@param[in] iLen 要跳过的比特数.不能是负数.
	 *	@return NULL.
	 *	@note 这里不作正确性判断.如果越界由Read,Write等其它
	 *			函数判断.
	 */
	void Skip(int iLen);

	/**
	 *	初始化对象.
	 *	@param[in]	pucData 指向数据区.
	 *	@param[in]	uiLen	数据区的字节数.
	 *	@return 
	 *	@remark	可以重复初始化.但不能不初始化.
	 */
    void Init(unsigned char* pucData, unsigned int uiLen) /*throw (IException)*/;
	/**
	 *	移动到下一字节开始位置.
	 *	若刚好在字节开始处则不移动.
	 *	@note
	 *		它不检查移动后的正确性.
	 */
	void Align(void);
	/**
	 *	检测下一位的值.
	 *	@return 是1返回1,是0返回0.
	 *	@note 速度慢了点.
	 */
	int Test(void);

	// 获取当前位在整个比特流中的索引.
	int GetCurPos(void);

	/**
	 *	判断是否到达比特序列结尾.
	 *	@return 到达最后返回ture,反之返回false.
	 */
	bool IsEnd(void);
	/**
	 * 无符号整数指数哥伦布码编码
	 */
	unsigned int get_ue_golomb() throw(IException);
	/**
	 * 有符号整数指数哥伦布码编码
	 */
	int get_se_golomb() throw(IException);
protected:
	bool				m_bInit;	//* 是否初始化标志.调用任何函数前必须保证其为true.
	unsigned char*		m_pStart;	//* 比特序列的起始位置.
	unsigned char*		m_pCur;		//* 比特序列的当前位置.
	unsigned char*		m_pEnd;		//* 比特序列的结束位置.
	int					m_iLeft;	//* 可访问的比特序列的位置索引.
	static unsigned int m_uiMask[33];	
	// 初始化内部成员
	void __Init(void);
};
}
#endif
