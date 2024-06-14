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

// A function adding two integers and returning the result

#include "stdafx.h"
#include "WebRtcSession.h"

DOM_CLASS_EXPORT_BEG
DOM_CLASS_EXPORT(CWebRtcSession,FILTER_TYPE_NAME,FT_Session,WEBRTC_PROTOCOL_NAME,MAKE_VERSION(0,0,0,0),(void*)CWebRtcSession::FilterQuery,NULL)
DOM_CLASS_EXPORT_END
