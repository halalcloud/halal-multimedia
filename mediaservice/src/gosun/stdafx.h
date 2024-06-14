#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <string>
#include <dirent.h>
#include <arpa/inet.h>
#include <memory>
#include <dom.h>
#include <iMulitimedia.h>
#include <iMediaService.h>
#include <Locker.h>
#include <Url.cpp>
#include <time.h>

using namespace std;

const char GOSUN_PROTOCOL_NAME[] = "gosun";
const uint16_t GOSUN_PORT = 8893;
const uint8_t MAX_PACKET_ID = 0xff;
int64_t GetTickCount();

#endif // STDAFX_H_INCLUDED

