#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <linux/videodev2.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <dom.h>
#include <iMulitimedia.h>
#include <sys/mman.h>
using namespace std;
const int INVALID_FD = -1;
extern int errno;

#endif // STDAFX_H_INCLUDED

