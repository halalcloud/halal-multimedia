#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED
#include <stdlib.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <list>
#include <memory>
#include <dom.h>
#include <iMulitimedia.h>
#include <Locker.h>
#include "../src/Url.cpp"

using namespace std;
extern int errno;

uint64_t GetMSCount();

#endif // STDAFX_H_INCLUDED
