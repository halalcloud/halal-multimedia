#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#include <vector>
#include <list>
#include <string>
#include <arpa/inet.h>
#include <memory>
#include <dom.h>
#include <iMulitimedia.h>
#include <iMediaService.h>
#include <Locker.h>
#include <Url.cpp>
using namespace std;

const char RTMP_PROTOCOL_NAME[] = "rtmp";
const char FLV_FORMAT_NAME[] = "flv";
const uint16_t RTMP_PORT = 1935;
const uint32_t flv_header_size = 13;
const char flv_header[flv_header_size] = { 'F', 'L', 'V', 0x01, 0x05, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00 };
#endif // STDAFX_H_INCLUDED

