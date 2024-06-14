#pragma once
#include <cppunit/extensions/HelperMacros.h>
#include "M2TSDemuxer.h"
using namespace wzd;
class CM2TSDemuxerTest
	:public CPPUNIT_NS::TestFixture
{

	CPPUNIT_TEST_SUITE(CM2TSDemuxerTest);	
	CPPUNIT_TEST(GetESPacketTest);
	CPPUNIT_TEST(FlushTest);
	CPPUNIT_TEST_SUITE_END();
	/*CPPUNIT_TEST(InitTest);
	CPPUNIT_TEST(InitTest2);*/
	/*CPPUNIT_TEST(GetESPacketTest2);
	CPPUNIT_TEST(GetPESPacketTest);
	CPPUNIT_TEST(GetPESPacketTest2);
	CPPUNIT_TEST(GetTimeStampTest);*/
public:
	CM2TSDemuxerTest(void);
	virtual ~CM2TSDemuxerTest(void);
public:
	void setUp();
	void tearDown();
	void GetESPacketTest();
	void FlushTest();

private:
	CM2TSDemuxer* m_demuxer;
	void __Init(std::ifstream& strm,int iLen);
	void __GetESPacket(std::ifstream& strmSrc, 
		std::ofstream& outA,
		std::ofstream& outV,
		unsigned short pid1, unsigned short pid2,
		int iLen, bool bPES = false);
	void __CreateES(std::string& str,		std::ofstream& outA,
		std::ofstream& outV);
};
