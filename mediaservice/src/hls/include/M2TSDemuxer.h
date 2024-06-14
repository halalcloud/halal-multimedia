#pragma once
#include "CTSDeMuxer.h"
namespace wzd
{
class CM2TSDemuxer
	:public CTSDeMuxer2
{
public:
	CM2TSDemuxer(void);
    virtual ~CM2TSDemuxer(void) throw();

        virtual int Init(const unsigned char* & buf, int iLen = 192 ) throw();
	virtual void Init(const Extra & strmProvider, _tstring & strInfo, int & iErrCode) throw();
	virtual int GetESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 192) throw(IException);
	virtual int GetESPacket(__int64 pos, Packet & pkt, const unsigned char* & buf, int iLen = 192) throw(IException);
	virtual int GetPESPacket(Packet & pkt, const unsigned char* & buf, int iLen = 192) throw(IException);
	virtual int GetESPacket(Packet & pkt, _tstring & strErrInfo, bool sync) throw();

	virtual bool GetTimeStamp(unsigned short& pid, long long & pts, 
		long long& dts, const unsigned char* buf, 
		int iLen = 188) throw();
	virtual bool Flush(Packet& pkt) ;
	virtual bool GetPCR( long long& pcr, unsigned short& pid,
		const unsigned char* buf, int iLen = 188);
private:
	/**
	 *	检测0x47
	 */
	bool __check(const unsigned char* & buf, int iLen) const;
	/**
	 *	用于解析额外的头.
	 */
	void __ParserHeader(const unsigned char* pbuf,int iLen);
};
}
