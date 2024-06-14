#ifndef WZD_IPACKETPARSER_H
#define WZD_IPACKETPARSER_H


#include "base.h"
namespace wzd {

/**
 * 解析器的接口.
 * 这个接口被架空了.
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
     * 构造函数.
     */
    IPacketParser() throw();

    /**
     * 虚拟析构函数.
     */
    virtual ~IPacketParser() throw();

    /**
     * 解析字节流.
     * 一次必须是一个TS包的大小.
     * @exception IException
     * @param[in] pBuf 数据包指针.
     * @param[in] 数据包大小.
     * @return 解析成功返回0,解析中返回1.
     */
    virtual int Parser(const void* & pBuf, int iBufLen) throw(IException) = 0;

    /**
     * 获取相关的信息.
     */
    virtual int GetInfo(void* & info, eInfos enmInfo) throw() = 0;


protected:
    /**
     * 检测数据的合法性.
     */
    virtual bool check() throw() = 0;


private:
    IPacketParser* m_parser;

    /**
     * psi解析器.
     */
    IPacketParser* m_psiParser;

};

} // namespace wzd
#endif
