#ifndef WZD_CTRANSPORTSTREAMPARSER_H
#define WZD_CTRANSPORTSTREAMPARSER_H


#include "base.h"

#include "mpeg2.h"
#include <map>
#define __use_array__


namespace wzd {

	struct pid_counter_table 
	{
		unsigned short pid;
		unsigned char cnt;
	};
/**
 * 解析一个个的TS包.
 * 对外提供TS所含有的信息.
 * 可以视情况试试并行计算.
 * 这里是TS解析的第一层,
 * 由于外部要获取的数据不清晰,只有将整个TS数据结构向外提供了.
 */
class CTransportStreamParser 
{
public:
    CTransportStreamParser() throw();

    /**
     * 构造函数
     */
    CTransportStreamParser(const void* & userData) throw();

    /**
     * 析构函数
     */
    virtual ~CTransportStreamParser();
    /**
     * 解析字节流.
     * 一次必须是一个TS包的大小.
     * @exception IException
     * @param[in] pBuf 数据包指针.
     * @param[in] 数据包大小.
     * @param[out] pid 本数据包的PID.
     * @return 解析失败返回0,解析成功返回数据偏移.
     */
    virtual int Parser(const unsigned char* & pBuf, int iBufLen, unsigned short & pid) throw(IException);

    /**
     * 重置,清除所有数据.
     */
	void ResetCounter() throw();
    /**
     * 获取TS数据头信息.
     * 只有解析成功才能调用此方法.
     * 否则是最近一次的解析数据.
     * 若从没有解析成功过,返回false.
     * @param[out] tsInfo TS流的数据结构.
     * @return 成功返回true,失败返回false.
     */
    inline bool GetTSInfo(TransportPacket & tsInfo) const throw();

    /**
     * 是否丢包.
     * 只有解析成功才能调用此方法.
     * 否则是最近一次的解析数据.
     * 若从没有解析成功过,返回false(从没有丢过包).
     * @return 丢包返回true,没有丢包返回false.
     */
    inline bool IsLostPackage() const throw();

    /**
     * 是否有附加字段.
     */
    inline bool HaveAdaptionField() const throw();
private:
#ifndef __use_array__
	/**
	 *	@param[in]	pid 当前包的PID
	 *	@param[in]	cnt	当前包的引用计数器.
	 *	@note	当第一次收到包时,就将包计数器变成其上一个值.这样能保证第一次收到的永远连续.
	 */
	map<unsigned short, unsigned char>::iterator __FindCounter(unsigned short pid, unsigned char cnt);
    /**
     * 最近一次的包计数器.用于丢包检测.
     */
    map<unsigned short, unsigned char> m_LastContinuityCounter;
	typedef pair <unsigned short, unsigned char> ContinuityPair;
#else
	pid_counter_table* __FindCounter(unsigned short pid, unsigned char cnt);
	pid_counter_table* m_counter;
	unsigned int		m_counterCnt;
#endif
    /**
     * ts包信息.用于存储当次解析出的数据.
     */
    TransportPacket m_tsPkt;
	bool			m_bContinuity;

};


/**
 * 获取TS数据头信息.
 * 只有解析成功才能调用此方法.
 * 否则是最近一次的解析数据.
 * 若从没有解析成功过,返回false.
 * @param[out] tsInfo TS流的数据结构.
 * @return 成功返回true,失败返回false.
 */
inline bool CTransportStreamParser::GetTSInfo(TransportPacket & tsInfo) const throw() 
{
	tsInfo = m_tsPkt;
	return true;
}

/**
 * 是否丢包.
 * 只有解析成功才能调用此方法.
 * 否则是最近一次的解析数据.
 * 若从没有解析成功过,返回false(从没有丢过包).
 * @return 丢包返回true,没有丢包返回false.
 */
inline bool CTransportStreamParser::IsLostPackage() const throw() 
{
	return !m_bContinuity;
}

/**
 * 是否有附加字段.
 */
inline bool CTransportStreamParser::HaveAdaptionField() const throw() 
{
  return (m_tsPkt.adaptationFieldControl == 0x2)||
	  (m_tsPkt.adaptationFieldControl == 0x3);

}
} // namespace wzd
#endif
