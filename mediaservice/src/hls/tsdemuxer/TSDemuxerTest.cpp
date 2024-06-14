#include <cassert>
#include <iostream>
#include "TSDemuxerTest.h"
#include "CTSDeMuxer.h"
#include "stdAfx.h"
#ifdef WINDOWS
#include <windows.h>
#endif
CPPUNIT_TEST_SUITE_REGISTRATION(CTSDemuxerTest);
CTSDemuxerTest::CTSDemuxerTest(void)
{
}

CTSDemuxerTest::~CTSDemuxerTest(void)
{
}

void CTSDemuxerTest::setUp()
{


}

void CTSDemuxerTest::tearDown()
{

}


void CTSDemuxerTest::FlushTest()
{

}

void CTSDemuxerTest::GetTimeStampTest()
{
	{
	m_demuxer2 = new wzd::CTSDeMuxer2;
	/* {{ h264, aac, 单音,单视. 测试
	std::ifstream	strmSrc;
	strmSrc.open(_T("D:\\hdnet-SD.ts"),
		std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	// 先构成PSI信息.
	{
		int iRet = -1;
		char buf[300];
		do 
		{
			const unsigned char* pBuf = (unsigned char*)buf;
			::memset(buf, 0, 188);
			strmSrc.read(buf, 188);
			if (strmSrc.good())
			{
				string strInfo;
				int iErrCode = -1;;
				iRet = m_demuxer2->Init(pBuf);
			}
			else
			{
				//* 文件读完都没有解析成功.
				break;;
			}
		} while (iRet);
		CPPUNIT_ASSERT(iRet == 0);
		CPPUNIT_ASSERT(m_demuxer2->GetProgramCount() == 0);
	}
	// 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);

	{
		bool bRet = false;
		char buf[300];
		bool bloop = false;
		int iLoop = 0;
		do 
		{
			//++iLoop;
			wzd::Packet pkt;
			const unsigned char* pBuf = (unsigned char*)buf;
			::memset(buf, 0, 300);
			strmSrc.read(buf, 188);
			if (bloop = strmSrc.good())
			{
				string strInfo;
				int iErrCode = -1;
				unsigned short pid;
				long long pts, dts;
				bRet = m_demuxer2->GetTimeStamp(pid, pts, dts, pBuf, 188);
				if (bRet)
				{
					std::cout<<"pid:"<<pid<<"\t"
						<<"pts:"<<pts<<"\t"
						<<"dts:"<<dts<<"\t"
						<<std::endl;
				}
			}
		} while (bloop);
		int iWaite = 0;
	}

	strmSrc.close();
	//* }} */
	delete m_demuxer2;
	m_demuxer = NULL;
	}
}

void CTSDemuxerTest::GetPESPacketTest()
{

}

void CTSDemuxerTest::GetPESPacketTest2()
{
	{
		m_demuxer = new wzd::CTSDeMuxer;
	//* {{ h264, aac, 单音,单视.
	std::ifstream	strmSrc;
    strmSrc.open("d:\\streaming\\698.ts", std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "ccut__50926_0_PES_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x44, 0x45, 188, true);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
}

void CTSDemuxerTest::GetESPacketTest()
{

}

void CTSDemuxerTest::GetESPacketTest2()
{
    unsigned int  dwBeg = GetTickCount();
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ h264, aac, 单音,单视. 测试
	std::ifstream	strmSrc;
	strmSrc.open(_T("D:\\Prob\\228.1.1.6_8001_2012-03-31_03-30-09-0781_live_cut.ts"),
		std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "test_mpts_CCTV_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x45, 0x44, 188);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ h264, aac, 单音,单视. hdnet
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\hdnet-SD.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "hdnet-SD_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x101, 0x100, 188);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ h264, aac, 单音,单视. edbox
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\edbox\\edbox-450k.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "edbox-450k_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 101, 100, 188);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ h264, aac, 单音,单视. envivio
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\envivio\\btv20111224.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "envivio_btv20111224_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x521, 0x529, 188);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ h264, aac, 单音,多视. envivio
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\envivio\\output-main.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "envivio_output-main_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x124, 0x131, 188);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ h264, aac, 单音,单视. ateme
	dwBeg = GetTickCount();
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\ateme\\ateme4m-001.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "ateme_ateme4m-001_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x101, 0x102, 188);
	outA.close();
	outV.close();
	strmSrc.close();
	std::cout<< GetTickCount() - dwBeg;
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ h264, aac, 单音,单视.
	TCHAR sz[] = _T("d:\\T_H.264_SD_4Mbps.ts");
	dwBeg = GetTickCount();
	std::ifstream	strmSrc;
	strmSrc.open(sz, std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "ccut__50926_0_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 188);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x100, 0x101, 188);
	outA.close();
	outV.close();
	strmSrc.close();
	std::wcout<<sz<<":"<<(GetTickCount() - dwBeg)<<"milliseconds"<<std::endl;
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ mpeg2, mp3, 多音,多视.
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\cctv\\cctva-10-10-23-44-00.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "cctv_cctva-10-10-23-44-00_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 204);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x120, 0x121, 204);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
	{
	m_demuxer = new wzd::CTSDeMuxer;
	/* {{ mpeg2, mp3, 多音,多视.
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\cctv\\cctv4-10-10-23-08-00.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	std::ofstream outA, outV;
	std::string str = "cctv_cctv4-10-10-23-08-00_";
	__CreateES(str, outA, outV);
	//* 先构成PSI信息.
	__Init(strmSrc, 204);
	//* 获取PES包.
	strmSrc.seekg(0, std::ios_base::beg);
	__GetESPacket(strmSrc, outA, outV, 0x11b, 0x11c, 204);
	outA.close();
	outV.close();
	strmSrc.close();
	//* }} */
	delete m_demuxer;
	m_demuxer = NULL;
	}
}

void CTSDemuxerTest::InitTest()
{
}

void CTSDemuxerTest::InitTest2()
{
#if 0
	{
	m_demuxer = new wzd::CTSDeMuxer;
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\ccut__50926_0.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	__Init(strmSrc,188);
	strmSrc.close();
	delete m_demuxer;
	m_demuxer = NULL;
	}

	{
	m_demuxer = new wzd::CTSDeMuxer;
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\cctv\\cctv.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	__Init(strmSrc,188);
	strmSrc.close();
	delete m_demuxer;
	m_demuxer = NULL;
	}

	{
	m_demuxer = new wzd::CTSDeMuxer;
	std::ifstream	strmSrc;
	strmSrc.open(_T("d:\\cctv\\cctv4-10-10-23-08-00.ts"), std::ios_base::binary | std::ios_base::in);
	assert(strmSrc.is_open());
	__Init(strmSrc, 204);
	strmSrc.close();
	delete m_demuxer;
	m_demuxer = NULL;
	}

	{
		m_demuxer = new wzd::CTSDeMuxer;
		std::ifstream	strmSrc;
		strmSrc.open(_T("d:\\cctv\\cctva-10-10-23-44-00.ts"), std::ios_base::binary | std::ios_base::in);
		assert(strmSrc.is_open());
		__Init(strmSrc, 204);
		strmSrc.close();
		delete m_demuxer;
		m_demuxer = NULL;
	}
#endif
}

void CTSDemuxerTest::__Init(std::ifstream& strm, int iLen)
{
	int iRet = -1;
	char buf[300];
	do 
	{
		const unsigned char* pBuf = (unsigned char*)buf;
		::memset(buf, 0, iLen);
		strm.read(buf, iLen);
		if (strm.good())
		{
			string strInfo;
			int iErrCode = -1;;
			iRet = m_demuxer->Init(pBuf);
		}
		else
		{
			//* 文件读完都没有解析成功.
			break;;
		}
	} while (iRet);
	CPPUNIT_ASSERT(iRet == 0);
	CPPUNIT_ASSERT(m_demuxer->GetProgramCount() == 0);
}

void CTSDemuxerTest::__GetESPacket(std::ifstream& strmSrc, std::ofstream& outA, std::ofstream& outV,
								unsigned short pid1, unsigned short pid2, int iLen, bool bPES /*= false*/)
{
	int iRet = -1;
	char buf[300];
	std::string strErrInfo;

	bool bloop = false;
	int iLoop = 0;
	int iLost = 0;
	do 
	{
		//++iLoop;
		wzd::Packet pkt;
		const unsigned char* pBuf = (unsigned char*)buf;
		::memset(buf, 0, 300);
		strmSrc.read(buf, iLen);
		if (bloop = strmSrc.good())
		{
			string strInfo;
			int iErrCode = -1;
			if (bPES)
			{
				iRet = m_demuxer->GetPESPacket(pkt, pBuf);
			}
			else
			{
				iRet = 	m_demuxer->GetESPacket(0, pkt, pBuf);
			}
			if (!iRet)
			{
#if 1
				if (pkt.pid == pid1)
				{
					outV.write((char*)pkt.pbuf, pkt.len);
					++iLoop;
				}
				else if (pkt.pid == pid2)
				{
					outA.write((char*)pkt.pbuf, pkt.len); // pkt.pbuf + pkt.len -16
					if (pkt.bContinue == false)
					{
						++iLost;
					}
				}
				else
				{
					;
				}
#endif
				m_demuxer->RecyclePacket(pkt);
			}
		}
		else
		{
			//* 文件读完都没有解析成功.
			break;;
		}
	} while (bloop);
	cout<<"Lost Packet Cnt:"<<iLost<<endl;
	int iWaite = 0;
}

void CTSDemuxerTest::__CreateES(std::string& str,		std::ofstream& outA,
				std::ofstream& outV)
{
	std::string strA, strV;
	strA = strV = str;
	strA += "Audio";
	strV += "Video";
	outA.open(strA.c_str(), std::ios_base::out | std::ios_base::binary);
	assert(outA.is_open());
	outV.open(strV.c_str(), std::ios_base::out | std::ios_base::binary);
	assert(outV.is_open());
}
