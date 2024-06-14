#ifndef WZD_CELEMENTSTREAMPARSER_H
#define WZD_CELEMENTSTREAMPARSER_H


#include "base.h"
#include "mpeg2.h"

namespace wzd {

/**
 * ����PES��ES��Ϣ.
 */
class CElementStreamParser 
{
public:
	CElementStreamParser();
	~CElementStreamParser();
    /**
     * �����ֽ���.
     * ������PESͷ��ʼ������
     * @exception IException ���ݴ����׳��쳣.
     * @param[in] pBuf ���ݰ�ָ��.
     * @param[in] ���ݰ���С.
     * @return ���ص�����������ƫ��
     */
    int Parser(const unsigned char* & pBuf, int iBufLen) throw(IException);

    /**
     * ��ȡ������.
     * @param[out] �����͵��ı�����.
     * @return �����͵�ö��.
     */
    inline eStreamType GetStreamType(
		_tstring & str) const throw();

    /**
     * �����������Ҫ��Щ����,ֻ��ȫ���ṩ��.
     */
    inline PESPacket GetESInfo() const throw();
protected:
    /**
     * ������ݵĺϷ���.
     * ǰ�����ֽڱ�����0x000001
     */
    bool check(const unsigned char* & pBuf, int iBufLen) throw();
private:
    /**
     * pes�� ����
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
 * ��ȡ������.
 * @param[out] �����͵��ı�����.
 * @return �����͵�ö��.
 */
inline eStreamType CElementStreamParser::GetStreamType(
	_tstring & str) const throw() 
{
  return ST_H264;
}

/**
 * �����������Ҫ��Щ����,ֻ��ȫ���ṩ��.
 */
inline PESPacket CElementStreamParser::GetESInfo(
	) const throw() 
{
  return m_pes;
}
} // namespace wzd
#endif
