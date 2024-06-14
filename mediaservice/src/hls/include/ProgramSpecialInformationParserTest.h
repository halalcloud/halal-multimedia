#pragma once
#include <cppunit/extensions/HelperMacros.h>
#include "CProgramSpecialInformationParser.h"
class CProgramSpecialInformationParserTest
	:public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(CProgramSpecialInformationParserTest);
	CPPUNIT_TEST(PatParserTest);
	CPPUNIT_TEST(PmtParserTest);
	CPPUNIT_TEST_SUITE_END();
public:
	CProgramSpecialInformationParserTest(void);
	~CProgramSpecialInformationParserTest(void);
	void setUp();
	void tearDown();
	void PatParserTest();
	void PmtParserTest();
private:
		wzd::CProgramSpecialInformationParser* m_parser;
};
