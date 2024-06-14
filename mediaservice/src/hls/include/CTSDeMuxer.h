#ifndef WZD_CTSDEMUXER_H
#define WZD_CTSDEMUXER_H


#include "base.h"
#include "ITSDemuxer.h"
#include <vector>

#include "CElementStreamPacker.h"
#include "CTransportStreamParser.h"
#include "CProgramSpecialInformationParser.h"
namespace wzd {
	extern f_log		g_log;
#define __use_array__
/**
 * 当不够一个完整的TS包时,它会缓存.直到后续数据来到.
 */
class CTSDeMuxer : public ITSDemuxer 
{
public:
    CTSDeMuxer() throw();

    virtual ~CTSDeMuxer() throw();

    /**
     * 对数据进行分析.
     * 在获取到一帧ES时返回.
     * 这里根据需要组织输出需要的数据.(基本是将TS,PSI,ES的数据合并成一个结构)
     * @param[out] 一帧数据
     * @param[out] 错误文本信息.
     * @param[in] sync 是否必须获取完整帧
     * @return 错误码信息.
     */
    virtual int GetESPacket(Packet & pkt, _tstring & strErrInfo, bool sync) throw();
	virtual int GetPESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException);

    /**
     * 对数据进行分析.
     * 在获取到一帧ES时返回.
     * @exception 处理失败抛出异常.
     * @param[out] 一帧数据
     * @param[in] buf 数据区指针.固定188,192,204,208
     * @return 获取到完整帧返回0,正理中返回1.
     */
    virtual int GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException);
	virtual int GetESPacket(__int64 pos, Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException);
    /**
     * 将对象内部状态恢复到初始化状态.
	 * @note 仅将包计数器和PES打包器的缓冲区置空了.
	 *			其余信息不涉及.
     */
    virtual void Reset() throw();

    /**
     * 初始化.
     * 构造TS所需的表必须在这里完成.
     */
    virtual void Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw();

    /**
     * 初始化.
     * 构造TS所需的表必须在这里完成.
     */
    virtual int Init(const unsigned char* & buf,
		int iLen = 188) throw(IException);

    /**
     * 判断是否可以Seek
     * @return 可以返回真,反之返回假如.
     */
    virtual bool IsSeekAble() throw();

	virtual const std::vector<ProgramMapSection>& GetPmt()
	{
		return m_psiParser.GetPMT();
	}

    /**
     * 回收资源.
     * @param[in] pkt 被回收的资源指针.
     * @return 空.
     */
    virtual void RecyclePacket(Packet & pkt) throw();

    virtual int GetProgramNames(vector<_tstring> & strnames) throw();

    /**
     * 获取节目数.
     * @return 有则返回具体节目数目,没有返回0.出错返回负值.
     */
    virtual int GetProgramCount() throw();

	virtual int GetTSBitrate() const 
	{
		return m_iCalcBitRate;
	}
	void Reet();
protected:
	static int const PacketCountThreshold = 100000;
	void __AddPacker(unsigned short pid, unsigned short ownerID);
	void __ReleasePacker();
	int  __CreatePackers();
    /**
     * 初始化内部环境.
     */
    void __Init() throw(IException);

    /**
     * 读取一个包,供分析.
     * 它分析TS层的数据类型.即:PES还是SI信息.
     */
    void __ReadPacket() throw(IException);

    /**
     * 流提供者.
     */
    Extra m_strmProvider;

    /**
     * 一路音频/视频流就有一个对象与之对应.加快解析速率.
     */
#ifndef __use_array__ 
    vector<CElementStreamPacker*> m_packers;
#else
	CElementStreamPacker** m_packers;
#endif
    CTransportStreamParser m_tsParser;
    CProgramSpecialInformationParser m_psiParser;
	int			m_iPkeCnt; //* 包计数器.当收到PAT后读取指定数目的包还没有解析完全PMT,则认为解析完成.
	int			m_iPCRCnt; //* PCR计数器.根据两个PCR间的数据量计算总码率.
	long long		m_iCalcBitRate; //* 统计出的码率.
	long long		m_iPCRBase;
	int			m_iPCRExt;

};

/**
 * 为了遵守接口一但定下就不许修改的原则,才派生出来的.
 */
class CTSDeMuxer2 : virtual public CTSDeMuxer, virtual public ITSDemuxer2
{
public:
	CTSDeMuxer2(f_log log = NULL);
	virtual ~CTSDeMuxer2() throw ();
	virtual bool GetTimeStamp(unsigned short& pid, long long & pts, 
		long long& dts, const unsigned char* buf, 
		int iLen = 188) throw();
	virtual bool Flush(Packet& pkt) ;
	virtual bool GetPCR( long long& pcr, unsigned short& pid,
		const unsigned char* buf, int iLen = 188);
protected:
	unsigned int		m_uiFlushCnt;
};

} // namespace wzd
#endif
