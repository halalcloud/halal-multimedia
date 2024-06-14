#ifndef WZD_CELEMENTSTREAMPARSER_H
#define WZD_CELEMENTSTREAMPARSER_H


#include "base.h"
#include "mpeg2.h"

namespace wzd {

/**
 * 解析PES包ES信息.
 */
class CElementStreamParser 
{
public:
	CElementStreamParser();
	~CElementStreamParser();
    /**
     * 解析字节流.
     * 必须是PES头开始的数据
     * @exception IException 数据错误抛出异常.
     * @param[in] pBuf 数据包指针.
     * @param[in] 数据包大小.
     * @return 返回的是数据区的偏移
     */
    int Parser(const unsigned char* & pBuf, int iBufLen) throw(IException);

    /**
     * 获取流类型.
     * @param[out] 流类型的文本描述.
     * @return 流类型的枚举.
     */
    inline eStreamType GetStreamType(
		_tstring & str) const throw();

    /**
     * 不清楚到底需要哪些数据,只好全部提供了.
     */
    inline PESPacket GetESInfo() const throw();
protected:
    /**
     * 检测数据的合法性.
     * 前三个字节必须是0x000001
     */
    bool check(const unsigned char* & pBuf, int iBufLen) throw();
private:
    /**
     * pes包 数据
     */
    PESPacket m_pes;
#if 0
	static long long rotate;//*
	unsigned int m_uiFactor;
	long long m_LastDts;
#endif
	void __ProgramStreamMap(const CBits& bit);
	void __ProgramStreamDirectory(const CBits& bit);
	void __PrivateStream2(const CBits& bit);
	bool __IsNormal();
	bool __IsSpecial();
	bool __IsPadding();
};


/**
 * 获取流类型.
 * @param[out] 流类型的文本描述.
 * @return 流类型的枚举.
 */
inline eStreamType CElementStreamParser::GetStreamType(
	_tstring & str) const throw() 
{
  return ST_H264;
}

/**
 * 不清楚到底需要哪些数据,只好全部提供了.
 */
inline PESPacket CElementStreamParser::GetESInfo(
	) const throw() 
{
  return m_pes;
}
} // namespace wzd
#endif
