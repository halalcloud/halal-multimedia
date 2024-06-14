#include "stdAfx.h"
#include "M2TSDemuxerTest.h"
#include <cassert>
#include <iostream>
#include <fstream>
//CPPUNIT_TEST_SUITE_REGISTRATION(CM2TSDemuxerTest);
CM2TSDemuxerTest::CM2TSDemuxerTest(void)
{
}

CM2TSDemuxerTest::~CM2TSDemuxerTest(void)
{
}

void CM2TSDemuxerTest::setUp()
{


}

void CM2TSDemuxerTest::tearDown()
{

}


void CM2TSDemuxerTest::FlushTest()
{

}

void CM2TSDemuxerTest::GetESPacketTest()
{
    unsigned int dwBeg = GetTickCount();
	m_demuxer = new wzd::CM2TSDemuxer;
    char sz[] = ("e:\\a.m2ts");
	std::ifstream  stmSrc;
	stmSrc.open(sz,std::ios_base::binary|std::ios_base::in);
	assert(stmSrc.is_open());
	std::ofstream outA,outV;
	std::string strName = "outputTest1";
	__CreateES(strName,outA,outV);
	__Init(stmSrc,192);
	stmSrc.seekg(0,std::ios_base::beg);
	__GetESPacket(stmSrc,outA,outV,0x1100,0x1011,192);
	outA.close();
	outV.close();
	stmSrc.close();
    std::wcout<<sz<<":"<<(GetTickCount() - dwBeg)<<"millisecondes"<<std::endl;
	delete m_demuxer;
	m_demuxer = NULL;
}

void CM2TSDemuxerTest::__Init(std::ifstream& strm,int iLen)
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
}

void CM2TSDemuxerTest::__GetESPacket(std::ifstream& strmSrc, std::ofstream& outA, std::ofstream& outV, unsigned short pid1, unsigned short pid2, int iLen, bool bPES /* = false */)
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
//#if 0
			if (!iRet)
			{

				if (pkt.pid == pid2)
				{
					outV.write((char*)pkt.pbuf, pkt.len);
					++iLoop;
				}
				else if (pkt.pid == pid1)
				{
					char*p = new char[2];
					memset(p,0,2);
					memcpy(p,(char*)pkt.pbuf,2);
					assert(*p == 0x0b);
					assert(*(p+1) == 0x77);

					outA.write((char*)pkt.pbuf, pkt.len); // pkt.pbuf + pkt.len -16
					if (pkt.bContinue == false)
					{
						++iLost;
					}
				}
				else
				{

				}
//#endif
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
}

void CM2TSDemuxerTest::__CreateES(std::string& str, std::ofstream& outA, std::ofstream& outV)
{
	std::string strA,strV;
	strA = strV = str;
	strA += "audio";
	strV += "video";
	outA.open(strA.c_str(),std::ios_base::out|std::ios_base::binary);
	assert(outA.is_open());
	outV.open(strV.c_str(),std::ios_base::out|std::ios_base::binary);
	assert(outV.is_open());
}
