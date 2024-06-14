#ifndef WZD_IPACKETPARSER_H
#define WZD_IPACKETPARSER_H


#include "base.h"
namespace wzd {

/**
 * �������Ľӿ�.
 * ����ӿڱ��ܿ���.
 */
class IPacketParser 
{
public:
    enum eInfos 
    {
      INFO_PAT,
      INFO_PMT,
      INFO_CAT

    };

    /**
     * ���캯��.
     */
    IPacketParser() throw();

    /**
     * ������������.
     */
    virtual ~IPacketParser() throw();

    /**
     * �����ֽ���.
     * һ�α�����һ��TS���Ĵ�С.
     * @exception IException
     * @param[in] pBuf ���ݰ�ָ��.
     * @param[in] ���ݰ���С.
     * @return �����ɹ�����0,�����з���1.
     */
    virtual int Parser(const void* & pBuf, int iBufLen) throw(IException) = 0;

    /**
     * ��ȡ��ص���Ϣ.
     */
    virtual int GetInfo(void* & info, eInfos enmInfo) throw() = 0;


protected:
    /**
     * ������ݵĺϷ���.
     */
    virtual bool check() throw() = 0;


private:
    IPacketParser* m_parser;

    /**
     * psi������.
     */
    IPacketParser* m_psiParser;

};

} // namespace wzd
#endif
