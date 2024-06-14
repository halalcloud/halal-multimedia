#include "M2TSDemuxer.h"
#include <assert.h>
#define  TSFLAG  0x47
namespace wzd
{

	CM2TSDemuxer::CM2TSDemuxer(void)
	{
	}

    CM2TSDemuxer::~CM2TSDemuxer(void) throw()
	{
	}

    int CM2TSDemuxer::Init(const unsigned char* & buf, int iLen /* = 192 */) throw ()
	{		
		if (__check(buf,iLen))
		{
			const unsigned char* pBuf = buf +4;
			return CTSDeMuxer::Init(pBuf,188);	
		}
		return -1;
	}

	void CM2TSDemuxer::Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw()
	{
		return CTSDeMuxer::Init(strmProvider,strInfo,iErrCode);
	}

	int CM2TSDemuxer::GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen /* = 192 */) throw(IException)
	{		
		if (__check(buf,iLen))
		{
			const unsigned char* pTsBuf = buf + 4;
			return wzd::CTSDeMuxer::GetESPacket(pkt,pTsBuf);
		}
		return -1;
	}

	int CM2TSDemuxer::GetESPacket(__int64 pos, Packet & pkt, const unsigned char* & buf, int iLen /* = 192 */) throw(IException)
	{		
		if (__check(buf,iLen))
		{
			const unsigned char* pTsBuf = buf + 4;
			return wzd::CTSDeMuxer::GetESPacket(pos,pkt,pTsBuf,188);
		}
		return -1;
	}

	int CM2TSDemuxer::GetPESPacket(Packet & pkt, const unsigned char* & buf, int iLen /* = 192 */) throw(IException)
	{
		if (__check(buf, iLen))
		{
			const unsigned char* pBuf = buf + 4;
			return CTSDeMuxer::GetPESPacket(pkt,pBuf,188);
		}
		return -1;
	}

	int CM2TSDemuxer::GetESPacket(Packet & pkt, _tstring & strErrInfo, bool sync) throw()
	{
		return CTSDeMuxer::GetESPacket(pkt,strErrInfo,sync);		
	}

	bool CM2TSDemuxer::GetTimeStamp(unsigned short& pid, long long & pts, long long& dts, const unsigned char* buf, int iLen /* = 188 */)
    throw()
	{
		if (__check(buf,iLen))
		{
			const unsigned char* pBuf = buf + 4;
			return CTSDeMuxer2::GetTimeStamp(pid,pts,dts,pBuf,188);
		}
		return false;
	}

	bool CM2TSDemuxer::Flush(Packet& pkt)
	{
		return CTSDeMuxer2::Flush(pkt);
	}

	bool CM2TSDemuxer::GetPCR(long long& pcr, unsigned short& pid, const unsigned char* buf, int iLen /* = 188 */)
	{
		if (__check(buf,iLen))
		{
			const unsigned char* pBuf = buf + 4;
			return CTSDeMuxer2::GetPCR(pcr,pid,pBuf,188);
		}
		return false;
	}

	bool CM2TSDemuxer::__check(const unsigned char* & buf, int iLen) const
	{
		if (TSFLAG == *(buf + 4))
		{
			return true;
		}
		return false;
	}

	void CM2TSDemuxer::__ParserHeader(const unsigned char* pbuf,int iLen)
	{
		assert(0);
	}
}
