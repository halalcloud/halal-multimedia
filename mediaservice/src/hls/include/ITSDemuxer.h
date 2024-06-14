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
	* ����ӿ�
	*/
	class ITSDemuxer 
	{
	public:
		ITSDemuxer() throw();

		/**
		* ��������.
		*/
		virtual ~ITSDemuxer() throw();

		/**
		* ��ʼ��.
		* ����TS����ı�������������.
		*/
		virtual void Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw() = 0;

		/**
		* ��ʼ��.
		* ����TS����ı�������������.
		* @exception ����ʧ���׳��쳣.
		* @param[in] ������ָ��.���ȹ̶�Ϊ188,192,204,208
		* @return �ɹ�����0,�����з���1.
		*/
		virtual int Init(const unsigned char* & buf,
			int iLen = 188) throw(IException) = 0;

		/**
		* �������ڲ�״̬�ָ�����ʼ��״̬.
		* @note ��������������PES������Ļ������ÿ���.
		*			������Ϣ���漰.
		*/
		virtual void Reset() throw() = 0;

		/**
		* �����ݽ��з���.
		* �ڻ�ȡ��һ��ESʱ����.
		* @param[out] һ֡����
		* @param[out] �����ı���Ϣ.
		* @param[in] sync �Ƿ�����ȡ����֡
		* @return ��������Ϣ.
		*/
		virtual int GetESPacket(Packet & pkt, _tstring & strErrInfo, bool sync) throw() = 0;

		/**
		* �����ݽ��з���.
		* �ڻ�ȡ��һ��ESʱ����.
		* @exception ����ʧ���׳��쳣.
		* @param[out] һ֡����
		* @param[in] buf ������ָ��.�̶�188
		* @return ��ȡ������֡����0,�����з���1.
		* @note ���ڼ�����,����pkt.transportScramblingControl��
		*/
		virtual int GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException) = 0;
		virtual int GetESPacket(__int64 pos, Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException) = 0;
		virtual int GetPESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException) = 0;

		/**
		* ������Դ.
		* @param[in] pkt �����յ���Դָ��.
		* @return ��.
		* @exception 
		*/
		virtual void RecyclePacket(Packet & pkt) throw() = 0;

		/**
		* ��ȡ��Ŀ��.
		* @return ���򷵻ؾ����Ŀ��Ŀ,û�з���0.�����ظ�ֵ.
		*/
		virtual int GetProgramCount() throw() = 0;

		virtual int GetProgramNames(vector<_tstring> & strnames) throw() = 0;

		/**
		* �ж��Ƿ����Seek
		* @return ���Է�����,��֮���ؼ���.
		*/
		virtual bool IsSeekAble() throw() = 0;
		virtual const std::vector<ProgramMapSection>& GetPmt() = 0;
		virtual int GetTSBitrate() const = 0;

	};

	class ITSDemuxer2
	{
	public:
		/**
		*	��ȡTS����PES��ͷ�е�ʱ���.
		*/
		virtual bool GetTimeStamp(unsigned short& pid, long long & pts, 
			long long& dts, const unsigned char* buf, 
			int iLen = 188) throw() = 0;
		/**
		*	��δ��ɵİ�ǿ��ˢ�³���.
		*	@param[out]
		*	@return �ɹ�����true;ʧ�ܷ���false;
		*/
		virtual bool Flush(Packet& pkt) = 0;

		virtual bool GetPCR( long long& pcr, unsigned short& pid, const unsigned char* buf, int iLen = 188) = 0;
	};
	typedef void (*f_log)( void * unused , int i_level, const char *psz_fmt, ... );
	ITSDemuxer* CreateTSDemuxer(int size = 188, f_log log = NULL);
	void DeleteTSDemuxer(ITSDemuxer*& pDemuxer);

} // namespace wzd
#endif
