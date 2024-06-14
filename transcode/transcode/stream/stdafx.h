#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED
#include <stdlib.h>
#include <malloc.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <sys/epoll.h>
#include <list>
#include <dom.h>
#include <iStream.h>

using namespace std;
extern int errno;
const int INVALID_FD = -1;

int64_t GetTickCount();

#endif // STDAFX_H_INCLUDED
