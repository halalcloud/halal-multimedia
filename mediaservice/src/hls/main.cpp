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
#include "HttpSession.h"
#include "TsDemuxer.h"
#include "TsMuxer.h"
#include <Locker.cpp>

DOM_CLASS_EXPORT_BEG
DOM_CLASS_EXPORT(CHttpSession,FILTER_TYPE_NAME,FT_Session,HTTP_PROTOCOL_NAME,MAKE_VERSION(0,0,0,0),(void*)CHttpSession::FilterQuery,(void*)CHttpSession::CreateListener)
DOM_CLASS_EXPORT(CTsDemuxer,FILTER_TYPE_NAME,FT_Transform,"ts format demuxer",MAKE_VERSION(0,0,0,0),(void*)CTsDemuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT(CTsMuxer,FILTER_TYPE_NAME,FT_Transform,"ts format muxer",MAKE_VERSION(0,0,0,0),(void*)CTsMuxer::FilterQuery,NULL)
DOM_CLASS_EXPORT_END
