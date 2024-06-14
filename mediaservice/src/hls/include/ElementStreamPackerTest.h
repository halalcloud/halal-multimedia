#pragma once
#include <cppunit/extensions/HelperMacros.h>
class CElementStreamPackerTest
	:public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(CElementStreamPackerTest);
	CPPUNIT_TEST(GetPacketTest);
	CPPUNIT_TEST_SUITE_END();
public:
	CElementStreamPackerTest(void);
	~CElementStreamPackerTest(void);
	void setUp();
	void tearDown();
	void GetPacketTest();
};
