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
//#include "file.h"
#include "network.h"
#include "buffer.h"
#include "BufSession.h"
#include "epoll.h"
#include <signal.h>

DOM_CLASS_EXPORT_BEG
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if(sigemptyset(&sa.sa_mask) == -1 || sigaction(SIGPIPE, &sa, 0) == -1)
    {
        perror("failed to ignore SIGPIPE; sigaction");
        return false;
    }
    DOM_CLASS_EXPORT(CEpoll,"epoll",MAKE_VERSION(0,0,0,0),NULL)
    //DOM_CLASS_EXPORT(CFileStream,"stream",MAKE_VERSION(0,0,0,0),NULL)
    DOM_CLASS_EXPORT(CNetworkStream,"stream",MAKE_VERSION(0,0,0,0),NULL)
    DOM_CLASS_EXPORT(CNetworkServer,"stream_server",MAKE_VERSION(0,0,0,0),NULL)
    DOM_CLASS_EXPORT(CBuffer,"Buffer",MAKE_VERSION(0,0,0,0),NULL)
    DOM_CLASS_EXPORT(CBufSession,"Session",MAKE_VERSION(0,0,0,0),NULL)
DOM_CLASS_EXPORT_END

int64_t GetTickCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 10000000 + ts.tv_nsec / 100;
}
