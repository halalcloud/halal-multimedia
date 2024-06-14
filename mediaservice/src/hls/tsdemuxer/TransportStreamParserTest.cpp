#include "stdAfx.h"
#include "TransportStreamParserTest.h"
#include "CTransportStreamParser.h"
CPPUNIT_TEST_SUITE_REGISTRATION(CTransportStreamParserTest);
CTransportStreamParserTest::CTransportStreamParserTest(void)
{
}

CTransportStreamParserTest::~CTransportStreamParserTest(void)
{
}

void CTransportStreamParserTest::setUp()
{

}

void CTransportStreamParserTest::tearDown()
{

}

void CTransportStreamParserTest::ParserTest()
{
	unsigned short pid;
	pid = 0xffff;
	int iRet = 0;
	wzd::CTransportStreamParser parser;
	//* {{ 解析有adaptation_field的TS
	{
		unsigned char buf[] = {0x47,0x41,0x00,0x30,0x07,0x10,0x00,0x00,0x74,0x04,0x00,0x00,0x00,0x00,0x01,0xE0,0xAD,0xBD,0x80,0xC0,0x0A,0x31,0x00,0x07,0xD8,0x61,0x11,0x00,0x07,0xBC,0x41,0x00,0x00,0x00,0x01,0x09,0xF0,0x00,0x00,0x00,0x01,0x67,0x4D,0x40,0x1F,0x9A,0x72,0x81,0x40,0x7B,0x7F,0xE0,0x00,0x80,0x00,0x62,0x00,0x00,0x03,0x00,0x02,0x00,0x00,0x03,0x00,0x64,0x1E,0x30,0x62,0x2C,0x00,0x00,0x00,0x01,0x68,0xE9,0x23,0x2C,0x80,0x00,0x00,0x01,0x06,0x05,0xFF,0xFF,0x8B,0xDC,0x45,0xE9,0xBD,0xE6,0xD9,0x48,0xB7,0x96,0x2C,0xD8,0x20,0xD9,0x23,0xEE,0xEF,0x78,0x32,0x36,0x34,0x20,0x2D,0x20,0x63,0x6F,0x72,0x65,0x20,0x31,0x30,0x30,0x20,0x2D,0x20,0x48,0x2E,0x32,0x36,0x34,0x2F,0x4D,0x50,0x45,0x47,0x2D,0x34,0x20,0x41,0x56,0x43,0x20,0x63,0x6F,0x64,0x65,0x63,0x20,0x2D,0x20,0x43,0x6F,0x70,0x79,0x6C,0x65,0x66,0x74,0x20,0x32,0x30,0x30,0x33,0x2D,0x32,0x30,0x31,0x30,0x20,0x2D,0x20,0x68,0x74,0x74,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x76,0x69,0x64,0x65,0x6F,0x6C,0x61,0x6E,0x2E,0x6F};
		const unsigned char* pByte = buf;
		CPPUNIT_ASSERT(12 == parser.Parser(pByte, sizeof(buf), pid));
		wzd::TransportPacket tsInfo;
		CPPUNIT_ASSERT(parser.GetTSInfo(tsInfo));
		CPPUNIT_ASSERT(tsInfo.syncByte == 0x47);
		CPPUNIT_ASSERT(0 == tsInfo.transportErrorIndicator);
		CPPUNIT_ASSERT(1 == tsInfo.payloadUnitStartIndicator);
		CPPUNIT_ASSERT(0 == tsInfo.transportPriority);
		CPPUNIT_ASSERT(0x100 == tsInfo.pid);
		CPPUNIT_ASSERT(0 == tsInfo.transportScramblingControl);
		CPPUNIT_ASSERT(0x3 == tsInfo.adaptationFieldControl);
		CPPUNIT_ASSERT(0 == tsInfo.continuityCounter);
		CPPUNIT_ASSERT(false == parser.IsLostPackage());
		CPPUNIT_ASSERT(parser.HaveAdaptionField());
		CPPUNIT_ASSERT(pid == 0x100);
		wzd::TransportPacket::adaptation_field& adpt = tsInfo.AdaptationField;
		CPPUNIT_ASSERT(0x7 == adpt.adaptation_field_length);
		CPPUNIT_ASSERT(0 == adpt.discontinuity_indicator);
		CPPUNIT_ASSERT(0 == adpt.random_access_indicator);
		CPPUNIT_ASSERT(0 == adpt.elementary_stream_priority_indicator);
		CPPUNIT_ASSERT(0 == adpt.OPCR_flag);
		CPPUNIT_ASSERT(0 == adpt.splicing_point_flag);
		CPPUNIT_ASSERT(0 == adpt.transport_private_data_flag);
		CPPUNIT_ASSERT(0 == adpt.adaptation_field_extension_flag);
		CPPUNIT_ASSERT(1 == adpt.PCR_flag);
		CPPUNIT_ASSERT(59400 == adpt.program_clock_reference_base);
		CPPUNIT_ASSERT(0 == adpt.program_clock_reference_extension);

		unsigned char buf1[] = {0x47,0x01,0x00,0x11,0x72,0x67,0x2F,0x78,0x32,0x36,0x34,0x2E,0x68,0x74,0x6D,0x6C,0x20,0x2D,0x20,0x6F,0x70,0x74,0x69,0x6F,0x6E,0x73,0x3A,0x20,0x63,0x61,0x62,0x61,0x63,0x3D,0x31,0x20,0x72,0x65,0x66,0x3D,0x34,0x20,0x64,0x65,0x62,0x6C,0x6F,0x63,0x6B,0x3D,0x31,0x3A,0x30,0x3A,0x30,0x20,0x61,0x6E,0x61,0x6C,0x79,0x73,0x65,0x3D,0x30,0x78,0x31,0x3A,0x30,0x78,0x31,0x31,0x31,0x20,0x6D,0x65,0x3D,0x75,0x6D,0x68,0x20,0x73,0x75,0x62,0x6D,0x65,0x3D,0x36,0x20,0x70,0x73,0x79,0x3D,0x31,0x20,0x70,0x73,0x79,0x5F,0x72,0x64,0x3D,0x31,0x2E,0x30,0x30,0x3A,0x30,0x2E,0x30,0x30,0x20,0x6D,0x69,0x78,0x65,0x64,0x5F,0x72,0x65,0x66,0x3D,0x30,0x20,0x6D,0x65,0x5F,0x72,0x61,0x6E,0x67,0x65,0x3D,0x31,0x36,0x20,0x63,0x68,0x72,0x6F,0x6D,0x61,0x5F,0x6D,0x65,0x3D,0x31,0x20,0x74,0x72,0x65,0x6C,0x6C,0x69,0x73,0x3D,0x31,0x20,0x38,0x78,0x38,0x64,0x63,0x74,0x3D,0x30,0x20,0x63,0x71,0x6D,0x3D,0x30,0x20,0x64,0x65,0x61,0x64,0x7A,0x6F,0x6E,0x65,0x3D,0x32,0x31,0x2C,0x31,0x31,0x20};
		const unsigned char* pByte1 = buf1;
		CPPUNIT_ASSERT(4 == parser.Parser(pByte1, sizeof(buf1), pid));
		CPPUNIT_ASSERT(parser.GetTSInfo(tsInfo));
		CPPUNIT_ASSERT(tsInfo.syncByte == 0x47);
		CPPUNIT_ASSERT(0 == tsInfo.transportErrorIndicator);
		CPPUNIT_ASSERT(0 == tsInfo.payloadUnitStartIndicator);
		CPPUNIT_ASSERT(0 == tsInfo.transportPriority);
		CPPUNIT_ASSERT(0x100 == tsInfo.pid);
		CPPUNIT_ASSERT(0 == tsInfo.transportScramblingControl);
		CPPUNIT_ASSERT(0x1 == tsInfo.adaptationFieldControl);
		CPPUNIT_ASSERT(1 == tsInfo.continuityCounter);
		CPPUNIT_ASSERT(false == parser.IsLostPackage());
		CPPUNIT_ASSERT(false == parser.HaveAdaptionField());
		CPPUNIT_ASSERT(pid == 0x100);

		unsigned char buf2[] = {0x47,0x01,0x00,0x18,0x1B,0xD7,0x09,0x9D,0x72,0x31,0x38,0x2C,0xF0,0x3D,0x42,0x5B,0xF7,0xDE,0x5E,0x12,0x52,0xD9,0xD8,0xFB,0xD2,0x55,0x1E,0x66,0xE9,0xA3,0x34,0x6F,0xAA,0xAA,0xAC,0x2A,0xF3,0x62,0xDC,0x2A,0xAE,0x75,0x28,0xC2,0x8F,0x51,0x48,0x6B,0xA1,0x39,0x34,0x73,0x29,0x5D,0x59,0x2E,0xF0,0x22,0x6F,0x40,0x23,0xBA,0x84,0x11,0x4F,0x0E,0xA0,0xF7,0x40,0x12,0x02,0x3D,0x0B,0x8D,0x02,0x3A,0xD0,0x9E,0x20,0xD7,0xC7,0x60,0x3B,0x76,0x23,0xC9,0xA8,0x6F,0x99,0xF6,0xF3,0x4D,0x59,0x01,0x61,0xA8,0xAE,0xB2,0xFE,0x58,0x2B,0xEF,0x53,0xC8,0x15,0x76,0xDE,0xFE,0x0A,0x0C,0x54,0x14,0xCF,0x3F,0xC6,0x66,0xB8,0xDB,0x45,0xF5,0x57,0x63,0xAA,0x5C,0x91,0x5D,0x88,0x57,0x2F,0x32,0x2B,0x98,0x0C,0x0C,0x5B,0xCF,0x2B,0xF3,0x97,0xB9,0x5C,0x44,0x18,0xF4,0x85,0x24,0x33,0x43,0xCD,0xD6,0xEE,0xD1,0xF5,0x6F,0x74,0x81,0xF7,0x7F,0xC4,0x55,0xE6,0xDB,0xF2,0x96,0xCD,0x1E,0x2D,0xA0,0x5D,0xE8,0xE2,0x3E,0x3C,0x02,0xB7,0x53,0x5F,0xEC,0x2D,0x3E,0xD4,0xFF,0x6D,0xA7,0xEE,0x1C,0xA0,0x8D};
		const unsigned char* pByte2 = buf2;
		CPPUNIT_ASSERT(4 == parser.Parser(pByte2, sizeof(buf2), pid));	
		CPPUNIT_ASSERT(true == parser.IsLostPackage());
		CPPUNIT_ASSERT(false == parser.HaveAdaptionField());
	}
	//* }}
	pid = 0xffff;
	//* {{ 解析没有adaptation_field的TS.
	{
		unsigned char buf[] = {0x47,0x40,0x00,0x10,0x00,0x00,0xB0,0x0D,0x00,0x01,0xC1,0x00,0x00,0x00,0x01,0xEF,0xFF,0x36,0x90,0xE2,0x3D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		const unsigned char * pByte = buf;
		CPPUNIT_ASSERT(4 == parser.Parser(pByte, sizeof(buf), pid));
		wzd::TransportPacket tsInfo;
		CPPUNIT_ASSERT(parser.GetTSInfo(tsInfo));
		CPPUNIT_ASSERT(tsInfo.syncByte == 0x47);
		CPPUNIT_ASSERT(0 == tsInfo.transportErrorIndicator);
		CPPUNIT_ASSERT(1 == tsInfo.payloadUnitStartIndicator);
		CPPUNIT_ASSERT(0 == tsInfo.transportPriority);
		CPPUNIT_ASSERT(0x00 == tsInfo.pid);
		CPPUNIT_ASSERT(0 == tsInfo.transportScramblingControl);
		CPPUNIT_ASSERT(0x1 == tsInfo.adaptationFieldControl);
		CPPUNIT_ASSERT(0 == tsInfo.continuityCounter);
		CPPUNIT_ASSERT(false == parser.IsLostPackage());
		CPPUNIT_ASSERT(false == parser.HaveAdaptionField());
		CPPUNIT_ASSERT(pid == 0x00);
	}
}
