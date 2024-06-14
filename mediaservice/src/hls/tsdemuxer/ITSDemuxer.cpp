
#include <cassert>
#include "ITSDemuxer.h"
#include "CTSDeMuxer.h"
#include "M2TSDemuxer.h"
namespace wzd {

ITSDemuxer::ITSDemuxer() throw() 
{

}

/**
 * Îö¹¹º¯Êý.
 */
ITSDemuxer::~ITSDemuxer() throw() 
{

}


ITSDemuxer* CreateTSDemuxer(int size /*= 188*/, f_log log /*= NULL*/)
{
	switch (size)
	{
	case 188:
		return new CTSDeMuxer2(log);
	case 192:
		return new CM2TSDemuxer;
	default:
		assert(0);
		return NULL;
	}
	return NULL;
}

void DeleteTSDemuxer(ITSDemuxer*& pDemuxer)
{
	if (pDemuxer)
	{
		delete pDemuxer;
		pDemuxer = NULL;
	}
}
 
} // namespace wzd
