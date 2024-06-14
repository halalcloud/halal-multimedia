#pragma once
#include <cppunit/extensions/HelperMacros.h>
class CTransportStreamParserTest
	:public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(CTransportStreamParserTest);
	CPPUNIT_TEST(ParserTest);
	CPPUNIT_TEST_SUITE_END();
public:
	CTransportStreamParserTest(void);
	~CTransportStreamParserTest(void);
	void setUp();
	void tearDown();
	void ParserTest();

};
