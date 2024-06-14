#pragma once
#include <cppunit/extensions/HelperMacros.h>
class CElementStreamParserTest
	:public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(CElementStreamParserTest);
	CPPUNIT_TEST(ParserTest);
	CPPUNIT_TEST_SUITE_END();
public:
	CElementStreamParserTest(void);
	~CElementStreamParserTest(void);
	void setUp();
	void tearDown();
	void ParserTest();
};
