#ifndef WZD_CPROGRAMSPECIALINFORMATIONPARSER_H
#define WZD_CPROGRAMSPECIALINFORMATIONPARSER_H


#include "base.h"

#include <vector>

#include "mpeg2.h"

namespace wzd {
//class CTSDeMuxer;
/**
 * PSI������.
 * ������ʽ������PSI/SI��Ϣ.
 * ��ȡ����������Ҫ�ṩ.
 */
class CProgramSpecialInformationParser 
{
public:
    CProgramSpecialInformationParser() throw();

    virtual ~CProgramSpecialInformationParser() throw();

    /**
     * �����ֽ���.
     * һ�α�����һ��TS���ĸ���.
     * ��ΪPMT��PATȫ������������,���Ը���PID��֪����ʲô����
     * @exception IException
     * @param[in] pBuf ���ݰ�ָ��.
     * @param[in] ���ݰ���С.
     * @param[in] pid ���ݰ���PIDֵ.
     * @return �����ɹ�����0,�����з���1.
     */
    virtual int Parser(const unsigned char* & pBuf, int iBufLen, unsigned short pid) throw(IException);
	inline bool IsPATFinshed();
	bool IsPSI(unsigned short pid);

    /**
     * ��ȡPat��.
     * �ⲿ�����޸�������.
     * ��ִ���´ν���֮ǰ���ݿ���.ִ�й�����֮�����ݻ�仯.
     */
    bool GetPAT(vector<ProgramAssociationSection> & pat) throw();

    /**
     * ��ȡPmt��.
     * �ⲿ�����޸�������.
     * ��ִ���´ν���֮ǰ���ݿ���.ִ�й�����֮�����ݻ�仯.
     */
    const vector<ProgramMapSection>& GetPMT() const throw();


protected:
    /**
     * ������ݵĺϷ���.
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
     * ԭʼPAT��.
     */
    vector<ProgramAssociationSection> m_pat;

    /**
     * pmt��.
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
