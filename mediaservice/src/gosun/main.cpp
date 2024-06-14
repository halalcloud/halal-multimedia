// The functions contained in this file are pretty dummy
// and are included only as a placeholder. Nevertheless,
// they *will* get included in the shared library if you
// don't remove them :)
//
// Obviously, you 'll have to write yourself the super-duper
// functions to include in the resulting library...
// Also, it's not necessary to write every function in this file.
// Feel free to add more files in this project. They will be
// included in the resulting library.

#include "stdafx.h"
#include "GosunSession.h"
#include "GosunDemuxer.h"
#include "GosunMuxer.h"
#include <Locker.cpp>

DOM_CLASS_EXPORT_BEG
DOM_CLASS_EXPORT(CGosunSession,FILTER_TYPE_NAME,FT_Session,GOSUN_PROTOCOL_NAME,MAKE_VERSION(0,0,0,0),(void*)CGosunSession::FilterQuery,(void*)CGosunSession::CreateListener)
DOM_CLASS_EXPORT(CGosunDemuxer,FILTER_TYPE_NAME,FT_Transform,"gosun format demuxer",MAKE_VERSION(0,0,0,0),(void*)CGosunDemuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT(CGosunMuxer,FILTER_TYPE_NAME,FT_Transform,"gosun format muxer",MAKE_VERSION(0,0,0,0),(void*)CGosunMuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT_END

int64_t GetTickCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec * 10000000 + ts.tv_nsec / 100;
}
