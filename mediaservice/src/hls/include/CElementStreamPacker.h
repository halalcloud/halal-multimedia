#ifndef WZD_CELEMENTSTREAMPACKER_H
#define WZD_CELEMENTSTREAMPACKER_H


#include "base.h"

#include "CElementStreamParser.h"
#include "mpeg2.h"

namespace wzd {

/**
 * ES包装器.将数据封装成指定格式的数据包或者对象.
 * 由于TS的一个包不是一个ES包,所以它必须缓存,等到完整数据后方可输出
 */
class CElementStreamPacker 
{
public:
    CElementStreamPacker() throw();

    ~CElementStreamPacker() throw();

    /**
     * 获取一个PES包.
     * @param[in] pBuf 数据区指针.
     * @param[in]	iBufLen 数据区长度.
	 * @paraam[in] bStartIndicator 是否包起始位.
     * @param[out] pkt 一包(无头PES包)数据.
     * @return 组成完整的一包(无头PES包)数据,返回0;反之返回1.
     * @exception IException 出错后抛出异常.
     */
    int GetPacket(Packet & pkt, const unsigned char* & pBuf, int iBufLen,
		bool bStartIndicator = false) throw(IException);
	int GetPacket(__int64 pos, Packet & pkt, const unsigned char* & pBuf, int iBufLen,
		bool bStartIndicator = false) throw(IException);
   /**
     * 获取一个带头PES包.
	 * @param[in] pBuf 数据区指针.
	 * @param[in]	iBufLen 数据区长度.
	 * @paraam[in] bStartIndicator 是否包起始位.
     * @param[out] pkt 一包(有头PES包)数据.
     * @return 组成完整的一包(有头PES包)数据,返回0;反之返回1.
     * @exception IException 出错后抛出异常.
     */
	int GetPESPacket(Packet & pkt, const unsigned char* & pBuf, int iBufLen,
		bool bStartIndicator = false) throw(IException);
    /**
     * 回收包.
     */
    inline void RecylePacket(Packet & pkt) throw();

	/**
	 *	设置加扰信息
	 */
	inline void SetScramblingMode(unsigned short us);

	/**
	 *	获取对象ID.
	 *	@return 对象ID.
	 */
	inline int GetID() const;
	/**
	 *	设置对象ID.
	 *	@param[in] id 对象ID.
	 */
	inline void SetID(unsigned short id);
	/**
	 *	获取对象拥有都ID.
	 *	@return 对象拥有者ID.
	 */
	inline int GetOwnerID() const;
	/**
	 *	设置对象拥有者(节目)ID.
	 *	@param[in] id 对象拥有者ID.
	 */
	inline void SetOwnerID(unsigned short id);
	/**
	 *	用于数据结束后,取出剩余缓冲区.
	 *	@param[out]	pkt 输出数据.
	 *	@return 成功返回true,失败返回false(没有数据).
	 */
	bool Flush(Packet& pkt);

	/**
	 *	清空缓冲区数据,重新接收数据
	 */
	void Reset();

	bool operator < (const CElementStreamPacker& obj) const;
private:
	/**
     * 填充输出结构并为后续工作作准备.
     */
	bool __Fill(Packet& pkt);
    /**
     * 解析Es.
     */
    CElementStreamParser m_esParser;

    /**
     * ES数据区.
     */
    unsigned char* m_data;

    /**
     * 数据区的总长度.
     */
    int m_uiDataLen;

    /**
     * 已使用的长度.
     */
    int m_usedLen;

    PESPacket m_pesInfo;
	unsigned short			m_id;
	unsigned short			m_ownerId;
	unsigned short			m_pesLen;
	/**
	 *	流的加扰模式
	 *	0表示没有加扰.非0表示加扰.
	 */
	unsigned short			m_usScramblingMode;
	__int64					m_pos;
};

inline void CElementStreamPacker::SetScramblingMode(unsigned short us)
{
	m_usScramblingMode = us;
}

inline int CElementStreamPacker::GetID() const
{
	return m_id;
}

inline void CElementStreamPacker::SetID(
	unsigned short id)
{
	m_id = id;
}

inline int CElementStreamPacker::GetOwnerID() const
{
	return m_ownerId;
}

inline void CElementStreamPacker::SetOwnerID(
	unsigned short id)
{
	m_ownerId = id;
}

/**
 * 回收包.
 */
inline void CElementStreamPacker::RecylePacket(
	Packet & pkt) throw() 
{
	delete [] pkt.pbuf;
	pkt.len = 0;
	pkt.pid = 0;
}
} // namespace wzd
#endif
