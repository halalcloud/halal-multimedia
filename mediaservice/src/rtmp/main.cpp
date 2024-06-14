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
#include "RtmpSession.h"
#include "FlvDemuxer.h"
#include "FlvMuxer.h"
#include <Locker.cpp>

DOM_CLASS_EXPORT_BEG
DOM_CLASS_EXPORT(CRtmpSession,FILTER_TYPE_NAME,FT_Session,RTMP_PROTOCOL_NAME,MAKE_VERSION(0,0,0,0),(void*)CRtmpSession::FilterQuery,(void*)CRtmpSession::CreateListener)
DOM_CLASS_EXPORT(CFlvDemuxer,FILTER_TYPE_NAME,FT_Transform,"flv format demuxer",MAKE_VERSION(0,0,0,0),(void*)CFlvDemuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFlvMuxer,FILTER_TYPE_NAME,FT_Transform,"flv format muxer",MAKE_VERSION(0,0,0,0),(void*)CFlvMuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT_END
