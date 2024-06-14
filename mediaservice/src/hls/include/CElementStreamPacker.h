#ifndef WZD_CELEMENTSTREAMPACKER_H
#define WZD_CELEMENTSTREAMPACKER_H


#include "base.h"

#include "CElementStreamParser.h"
#include "mpeg2.h"

namespace wzd {

/**
 * ES��װ��.�����ݷ�װ��ָ����ʽ�����ݰ����߶���.
 * ����TS��һ��������һ��ES��,���������뻺��,�ȵ��������ݺ󷽿����
 */
class CElementStreamPacker 
{
public:
    CElementStreamPacker() throw();

    ~CElementStreamPacker() throw();

    /**
     * ��ȡһ��PES��.
     * @param[in] pBuf ������ָ��.
     * @param[in]	iBufLen ����������.
	 * @paraam[in] bStartIndicator �Ƿ����ʼλ.
     * @param[out] pkt һ��(��ͷPES��)����.
     * @return ���������һ��(��ͷPES��)����,����0;��֮����1.
     * @exception IException ������׳��쳣.
     */
    int GetPacket(Packet & pkt, const unsigned char* & pBuf, int iBufLen,
		bool bStartIndicator = false) throw(IException);
	int GetPacket(__int64 pos, Packet & pkt, const unsigned char* & pBuf, int iBufLen,
		bool bStartIndicator = false) throw(IException);
   /**
     * ��ȡһ����ͷPES��.
	 * @param[in] pBuf ������ָ��.
	 * @param[in]	iBufLen ����������.
	 * @paraam[in] bStartIndicator �Ƿ����ʼλ.
     * @param[out] pkt һ��(��ͷPES��)����.
     * @return ���������һ��(��ͷPES��)����,����0;��֮����1.
     * @exception IException ������׳��쳣.
     */
	int GetPESPacket(Packet & pkt, const unsigned char* & pBuf, int iBufLen,
		bool bStartIndicator = false) throw(IException);
    /**
     * ���հ�.
     */
    inline void RecylePacket(Packet & pkt) throw();

	/**
	 *	���ü�����Ϣ
	 */
	inline void SetScramblingMode(unsigned short us);

	/**
	 *	��ȡ����ID.
	 *	@return ����ID.
	 */
	inline int GetID() const;
	/**
	 *	���ö���ID.
	 *	@param[in] id ����ID.
	 */
	inline void SetID(unsigned short id);
	/**
	 *	��ȡ����ӵ�ж�ID.
	 *	@return ����ӵ����ID.
	 */
	inline int GetOwnerID() const;
	/**
	 *	���ö���ӵ����(��Ŀ)ID.
	 *	@param[in] id ����ӵ����ID.
	 */
	inline void SetOwnerID(unsigned short id);
	/**
	 *	�������ݽ�����,ȡ��ʣ�໺����.
	 *	@param[out]	pkt �������.
	 *	@return �ɹ�����true,ʧ�ܷ���false(û������).
	 */
	bool Flush(Packet& pkt);

	/**
	 *	��ջ���������,���½�������
	 */
	void Reset();

	bool operator < (const CElementStreamPacker& obj) const;
private:
	/**
     * �������ṹ��Ϊ����������׼��.
     */
	bool __Fill(Packet& pkt);
    /**
     * ����Es.
     */
    CElementStreamParser m_esParser;

    /**
     * ES������.
     */
    unsigned char* m_data;

    /**
     * ���������ܳ���.
     */
    int m_uiDataLen;

    /**
     * ��ʹ�õĳ���.
     */
    int m_usedLen;

    PESPacket m_pesInfo;
	unsigned short			m_id;
	unsigned short			m_ownerId;
	unsigned short			m_pesLen;
	/**
	 *	���ļ���ģʽ
	 *	0��ʾû�м���.��0��ʾ����.
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
 * ���հ�.
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
