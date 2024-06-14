#pragma once
#include <cppunit/extensions/HelperMacros.h>
#include "CTSDeMuxer.h"
class CTSDemuxerTest
	:public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(CTSDemuxerTest);
	CPPUNIT_TEST(InitTest);
	CPPUNIT_TEST(InitTest2);
	CPPUNIT_TEST(GetESPacketTest);
	CPPUNIT_TEST(GetESPacketTest2);
	CPPUNIT_TEST(GetPESPacketTest);
	CPPUNIT_TEST(GetPESPacketTest2);
	CPPUNIT_TEST(GetTimeStampTest);
	CPPUNIT_TEST(FlushTest);
	CPPUNIT_TEST_SUITE_END();
public:
	CTSDemuxerTest(void);
	~CTSDemuxerTest(void);

	static std::string GetSuiteName()
	{
		return "CTSDemuxerTest";
	}

	void setUp();
	void tearDown();
	void GetESPacketTest();
	//* 每次操作一个.
	void GetESPacketTest2();
	void GetPESPacketTest();
	//* 每次操作一个.
	void GetPESPacketTest2();
	void InitTest();
	//* 每次操作一个.
	void InitTest2();
	void FlushTest();
	void GetTimeStampTest();
private:
	wzd::CTSDeMuxer* m_demuxer;
	wzd::CTSDeMuxer2* m_demuxer2;
	void __Init(std::ifstream& strm, int iLen);
	void __GetESPacket(std::ifstream& strmSrc, 
		std::ofstream& outA,
		std::ofstream& outV,
		unsigned short pid1, unsigned short pid2,
		int iLen, bool bPES = false);
	void __CreateES(std::string& str,		std::ofstream& outA,
		std::ofstream& outV);

};
