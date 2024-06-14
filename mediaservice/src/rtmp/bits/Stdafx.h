/////////////////////////////////////////////////////////////////////////////////////////////
// Project:Mp4Format
// Author:ChenZhong
// Date:星期五, 二月 17, 2012
// Description:The Mp4 Demuxer and Muxer Register
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __H_Mp4Format_STDAFX__
#define __H_Mp4Format_STDAFX__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <string.h>

#define ADTS_HEADER_LENGTH  7
typedef unsigned int DWORD;
typedef short WORD;
typedef short* LPWORD;
typedef char CHAR;
typedef unsigned int* LPDWORD;
typedef unsigned char BYTE;
typedef unsigned char* LPBYTE;
typedef unsigned int HRESULT;
typedef long long LONGLONG;
typedef unsigned long long UINT64;
typedef long long INT64;
typedef unsigned int UINT32;
typedef unsigned int UINT;
typedef int INT;
typedef void* LPVOID ;
typedef int BOOL;
typedef char* LPSTR;
typedef char* LPTSTR;
#define TRUE 1
#define FALSE 0
#define S_OK 0
#define MAX_PACKET_HEADER_SIZE (1024 * 20)
#define ADTS_HEADER_LENGTH  7
#define DSI_SIZE 10
#define E_UNEXPECTED -10
#define ERROR_NOT_ENOUGH_MEMORY -11
#define E_ABORT -12
#define E_TIMESTAMP -1
#define E_INVALID_ARG -13
#endif // __H_Mp4Format_STDAFX__
