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
#include "file.h"
#include "network.h"
#include "epoll.h"
#include <Locker.cpp>

DOM_CLASS_EXPORT_BEG
    DOM_CLASS_EXPORT(CEpoll,NULL,0,"epoll",MAKE_VERSION(0,0,0,0),NULL,NULL)
    DOM_CLASS_EXPORT(CFileStream,STREAM_TYPE_NAME,0,"file stream",MAKE_VERSION(0,0,0,0),NULL,NULL)
    DOM_CLASS_EXPORT(CNetworkStream,STREAM_TYPE_NAME,0,"tcp stream",MAKE_VERSION(0,0,0,0),NULL,NULL)
    DOM_CLASS_EXPORT(CNetworkListen,LISTEN_TYPE_NAME,0,"tcp stream listen",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT_END

uint64_t GetMSCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
