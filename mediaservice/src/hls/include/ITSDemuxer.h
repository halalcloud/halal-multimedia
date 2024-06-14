#ifndef WZD_ITSDEMUXER_H
#define WZD_ITSDEMUXER_H


#include "base.h"
#include "mpeg2.h"
#include <vector>


namespace wzd {

	/* Log level */
#define LOG_NONE          (-1)
#define LOG_ERROR          0
#define LOG_WARNING        1
#define LOG_INFO           2
#define LOG_DEBUG          3
	/**
	* 对外接口
	*/
	class ITSDemuxer 
	{
	public:
		ITSDemuxer() throw();

		/**
		* 析构函数.
		*/
		virtual ~ITSDemuxer() throw();

		/**
		* 初始化.
		* 构造TS所需的表必须在这里完成.
		*/
		virtual void Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw() = 0;

		/**
		* 初始化.
		* 构造TS所需的表必须在这里完成.
		* @exception 处理失败抛出异常.
		* @param[in] 数据区指针.长度固定为188,192,204,208
		* @return 成功返回0,处理中返回1.
		*/
		virtual int Init(const unsigned char* & buf,
			int iLen = 188) throw(IException) = 0;

		/**
		* 将对象内部状态恢复到初始化状态.
		* @note 仅将包计数器和PES打包器的缓冲区置空了.
		*			其余信息不涉及.
		*/
		virtual void Reset() throw() = 0;

		/**
		* 对数据进行分析.
		* 在获取到一包ES时返回.
		* @param[out] 一帧数据
		* @param[out] 错误文本信息.
		* @param[in] sync 是否必须获取完整帧
		* @return 错误码信息.
		*/
		virtual int GetESPacket(Packet & pkt, _tstring & strErrInfo, bool sync) throw() = 0;

		/**
		* 对数据进行分析.
		* 在获取到一包ES时返回.
		* @exception 处理失败抛出异常.
		* @param[out] 一帧数据
		* @param[in] buf 数据区指针.固定188
		* @return 获取到完整帧返回0,正理中返回1.
		* @note 对于加扰流,仅置pkt.transportScramblingControl项
		*/
		virtual int GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException) = 0;
		virtual int GetESPacket(__int64 pos, Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException) = 0;
		virtual int GetPESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException) = 0;

		/**
		* 回收资源.
		* @param[in] pkt 被回收的资源指针.
		* @return 空.
		* @exception 
		*/
		virtual void RecyclePacket(Packet & pkt) throw() = 0;

		/**
		* 获取节目数.
		* @return 有则返回具体节目数目,没有返回0.出错返回负值.
		*/
		virtual int GetProgramCount() throw() = 0;

		virtual int GetProgramNames(vector<_tstring> & strnames) throw() = 0;

		/**
		* 判断是否可以Seek
		* @return 可以返回真,反之返回假如.
		*/
		virtual bool IsSeekAble() throw() = 0;
		virtual const std::vector<ProgramMapSection>& GetPmt() = 0;
		virtual int GetTSBitrate() const = 0;

	};

	class ITSDemuxer2
	{
	public:
		/**
		*	获取TS包中PES包头中的时间戳.
		*/
		virtual bool GetTimeStamp(unsigned short& pid, long long & pts, 
			long long& dts, const unsigned char* buf, 
			int iLen = 188) throw() = 0;
		/**
		*	将未完成的包强制刷新出来.
		*	@param[out]
		*	@return 成功返回true;失败返回false;
		*/
		virtual bool Flush(Packet& pkt) = 0;

		virtual bool GetPCR( long long& pcr, unsigned short& pid, const unsigned char* buf, int iLen = 188) = 0;
	};
	typedef void (*f_log)( void * unused , int i_level, const char *psz_fmt, ... );
	ITSDemuxer* CreateTSDemuxer(int size = 188, f_log log = NULL);
	void DeleteTSDemuxer(ITSDemuxer*& pDemuxer);

} // namespace wzd
#endif
