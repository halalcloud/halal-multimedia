#ifndef WZD_CPROGRAMSPECIALINFORMATIONPARSER_H
#define WZD_CPROGRAMSPECIALINFORMATIONPARSER_H


#include "base.h"

#include <vector>

#include "mpeg2.h"

namespace wzd {
//class CTSDeMuxer;
/**
 * PSI解析器.
 * 解析各式各样的PSI/SI信息.
 * 获取方法根据需要提供.
 */
class CProgramSpecialInformationParser 
{
public:
    CProgramSpecialInformationParser() throw();

    virtual ~CProgramSpecialInformationParser() throw();

    /**
     * 解析字节流.
     * 一次必须是一个TS包的负载.
     * 因为PMT和PAT全部保存在类中,所以根据PID可知道是什么数据
     * @exception IException
     * @param[in] pBuf 数据包指针.
     * @param[in] 数据包大小.
     * @param[in] pid 数据包的PID值.
     * @return 解析成功返回0,解析中返回1.
     */
    virtual int Parser(const unsigned char* & pBuf, int iBufLen, unsigned short pid) throw(IException);
	inline bool IsPATFinshed();
	bool IsPSI(unsigned short pid);

    /**
     * 获取Pat表.
     * 外部不能修改其内容.
     * 在执行下次解析之前内容可用.执行过解析之后内容会变化.
     */
    bool GetPAT(vector<ProgramAssociationSection> & pat) throw();

    /**
     * 获取Pmt表.
     * 外部不能修改其内容.
     * 在执行下次解析之前内容可用.执行过解析之后内容会变化.
     */
    const vector<ProgramMapSection>& GetPMT() const throw();


protected:
    /**
     * 检测数据的合法性.
     */
    virtual bool check() throw();
private:
    int __PATParse(unsigned char* pBuf, int iBufLen) throw(IException);

    int __PMTParse(unsigned char* pBuf, int iBufLen) throw(IException) ;

    int __NITParse() throw();

    int __CATParse() throw();

    int __TSDTParse() throw();
	bool __IsPMT(unsigned short pid);
	bool __IsPatSectionExist(unsigned char section_number);
	bool __IsPmtSectionExist(unsigned short program_number);
	unsigned int __GetProgramCount();

    /**
     * 原始PAT表.
     */
    vector<ProgramAssociationSection> m_pat;

    /**
     * pmt表.
     */
    vector<ProgramMapSection> m_pmt;

	bool		m_bPatFinished;

};
inline bool CProgramSpecialInformationParser::IsPATFinshed()
{
	return m_bPatFinished;
}

} // namespace wzd
#endif
