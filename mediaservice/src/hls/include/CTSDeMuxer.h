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
 * ������һ��������TS��ʱ,���Ỻ��.ֱ��������������.
 */
class CTSDeMuxer : public ITSDemuxer 
{
public:
    CTSDeMuxer() throw();

    virtual ~CTSDeMuxer() throw();

    /**
     * �����ݽ��з���.
     * �ڻ�ȡ��һ֡ESʱ����.
     * ���������Ҫ��֯�����Ҫ������.(�����ǽ�TS,PSI,ES�����ݺϲ���һ���ṹ)
     * @param[out] һ֡����
     * @param[out] �����ı���Ϣ.
     * @param[in] sync �Ƿ�����ȡ����֡
     * @return ��������Ϣ.
     */
    virtual int GetESPacket(Packet & pkt, _tstring & strErrInfo, bool sync) throw();
	virtual int GetPESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException);

    /**
     * �����ݽ��з���.
     * �ڻ�ȡ��һ֡ESʱ����.
     * @exception ����ʧ���׳��쳣.
     * @param[out] һ֡����
     * @param[in] buf ������ָ��.�̶�188,192,204,208
     * @return ��ȡ������֡����0,�����з���1.
     */
    virtual int GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException);
	virtual int GetESPacket(__int64 pos, Packet & pkt, const unsigned char* & buf, int iLen = 188) throw(IException);
    /**
     * �������ڲ�״̬�ָ�����ʼ��״̬.
	 * @note ��������������PES������Ļ������ÿ���.
	 *			������Ϣ���漰.
     */
    virtual void Reset() throw();

    /**
     * ��ʼ��.
     * ����TS����ı�������������.
     */
    virtual void Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw();

    /**
     * ��ʼ��.
     * ����TS����ı�������������.
     */
    virtual int Init(const unsigned char* & buf,
		int iLen = 188) throw(IException);

    /**
     * �ж��Ƿ����Seek
     * @return ���Է�����,��֮���ؼ���.
     */
    virtual bool IsSeekAble() throw();

	virtual const std::vector<ProgramMapSection>& GetPmt()
	{
		return m_psiParser.GetPMT();
	}

    /**
     * ������Դ.
     * @param[in] pkt �����յ���Դָ��.
     * @return ��.
     */
    virtual void RecyclePacket(Packet & pkt) throw();

    virtual int GetProgramNames(vector<_tstring> & strnames) throw();

    /**
     * ��ȡ��Ŀ��.
     * @return ���򷵻ؾ����Ŀ��Ŀ,û�з���0.�����ظ�ֵ.
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
     * ��ʼ���ڲ�����.
     */
    void __Init() throw(IException);

    /**
     * ��ȡһ����,������.
     * ������TS�����������.��:PES����SI��Ϣ.
     */
    void __ReadPacket() throw(IException);

    /**
     * ���ṩ��.
     */
    Extra m_strmProvider;

    /**
     * һ·��Ƶ/��Ƶ������һ��������֮��Ӧ.�ӿ��������.
     */
#ifndef __use_array__ 
    vector<CElementStreamPacker*> m_packers;
#else
	CElementStreamPacker** m_packers;
#endif
    CTransportStreamParser m_tsParser;
    CProgramSpecialInformationParser m_psiParser;
	int			m_iPkeCnt; //* ��������.���յ�PAT���ȡָ����Ŀ�İ���û�н�����ȫPMT,����Ϊ�������.
	int			m_iPCRCnt; //* PCR������.��������PCR�������������������.
	long long		m_iCalcBitRate; //* ͳ�Ƴ�������.
	long long		m_iPCRBase;
	int			m_iPCRExt;

};

/**
 * Ϊ�����ؽӿ�һ�����¾Ͳ����޸ĵ�ԭ��,������������.
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
