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
 * ����һ������TS��.
 * �����ṩTS�����е���Ϣ.
 * ������������Բ��м���.
 * ������TS�����ĵ�һ��,
 * �����ⲿҪ��ȡ�����ݲ�����,ֻ�н�����TS���ݽṹ�����ṩ��.
 */
class CTransportStreamParser 
{
public:
    CTransportStreamParser() throw();

    /**
     * ���캯��
     */
    CTransportStreamParser(const void* & userData) throw();

    /**
     * ��������
     */
    virtual ~CTransportStreamParser();
    /**
     * �����ֽ���.
     * һ�α�����һ��TS���Ĵ�С.
     * @exception IException
     * @param[in] pBuf ���ݰ�ָ��.
     * @param[in] ���ݰ���С.
     * @param[out] pid �����ݰ���PID.
     * @return ����ʧ�ܷ���0,�����ɹ���������ƫ��.
     */
    virtual int Parser(const unsigned char* & pBuf, int iBufLen, unsigned short & pid) throw(IException);

    /**
     * ����,�����������.
     */
	void ResetCounter() throw();
    /**
     * ��ȡTS����ͷ��Ϣ.
     * ֻ�н����ɹ����ܵ��ô˷���.
     * ���������һ�εĽ�������.
     * ����û�н����ɹ���,����false.
     * @param[out] tsInfo TS�������ݽṹ.
     * @return �ɹ�����true,ʧ�ܷ���false.
     */
    inline bool GetTSInfo(TransportPacket & tsInfo) const throw();

    /**
     * �Ƿ񶪰�.
     * ֻ�н����ɹ����ܵ��ô˷���.
     * ���������һ�εĽ�������.
     * ����û�н����ɹ���,����false(��û�ж�����).
     * @return ��������true,û�ж�������false.
     */
    inline bool IsLostPackage() const throw();

    /**
     * �Ƿ��и����ֶ�.
     */
    inline bool HaveAdaptionField() const throw();
private:
#ifndef __use_array__
	/**
	 *	@param[in]	pid ��ǰ����PID
	 *	@param[in]	cnt	��ǰ�������ü�����.
	 *	@note	����һ���յ���ʱ,�ͽ����������������һ��ֵ.�����ܱ�֤��һ���յ�����Զ����.
	 */
	map<unsigned short, unsigned char>::iterator __FindCounter(unsigned short pid, unsigned char cnt);
    /**
     * ���һ�εİ�������.���ڶ������.
     */
    map<unsigned short, unsigned char> m_LastContinuityCounter;
	typedef pair <unsigned short, unsigned char> ContinuityPair;
#else
	pid_counter_table* __FindCounter(unsigned short pid, unsigned char cnt);
	pid_counter_table* m_counter;
	unsigned int		m_counterCnt;
#endif
    /**
     * ts����Ϣ.���ڴ洢���ν�����������.
     */
    TransportPacket m_tsPkt;
	bool			m_bContinuity;

};


/**
 * ��ȡTS����ͷ��Ϣ.
 * ֻ�н����ɹ����ܵ��ô˷���.
 * ���������һ�εĽ�������.
 * ����û�н����ɹ���,����false.
 * @param[out] tsInfo TS�������ݽṹ.
 * @return �ɹ�����true,ʧ�ܷ���false.
 */
inline bool CTransportStreamParser::GetTSInfo(TransportPacket & tsInfo) const throw() 
{
	tsInfo = m_tsPkt;
	return true;
}

/**
 * �Ƿ񶪰�.
 * ֻ�н����ɹ����ܵ��ô˷���.
 * ���������һ�εĽ�������.
 * ����û�н����ɹ���,����false(��û�ж�����).
 * @return ��������true,û�ж�������false.
 */
inline bool CTransportStreamParser::IsLostPackage() const throw() 
{
	return !m_bContinuity;
}

/**
 * �Ƿ��и����ֶ�.
 */
inline bool CTransportStreamParser::HaveAdaptionField() const throw() 
{
  return (m_tsPkt.adaptationFieldControl == 0x2)||
	  (m_tsPkt.adaptationFieldControl == 0x3);

}
} // namespace wzd
#endif
