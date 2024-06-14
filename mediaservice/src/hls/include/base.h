#ifndef WZD_BASE_H
#define WZD_BASE_H

/**
 * �������ݽṹ�����ﶨ��.
 */
#include "wzdexception.h"
#include <string>
#ifdef WIN32
#include <tchar.h>
#else
#define __int64 long long
#endif
#include <memory.h>



namespace wzd {
class CBits;
typedef std::string _tstring;
typedef void* Extra;
struct  Packet
{
	unsigned char* pbuf;
	unsigned int len;
	unsigned short pid;

	__int64	pts;
	__int64 dts;

	bool bContinue;					//* ���Ƿ�����
	__int64 pos;
	/**
	 *	���ļ���ģʽ
	 *	0��ʾû�м���.��0��ʾ����.
	 */
	unsigned short usScramblingMode;
};
enum eInfos 
{
  INFO_PAT,
  INFO_PMT,
  INFO_CAT

};
typedef void* sEsInfo;
/**
 * ��ý������ͱ�ʶ��.
 */
enum eStreamType 
{
  ST_H264,
  ST_AAC,
  ST_H263,
  ST_MP3,
  ST_MPEG4

};
extern __int64 Get33_1_32(CBits& bit);
extern __int64 Get33_3_15_15(CBits& bit);
DEFINE_EXCEPTION_CLASS(Mpegts);
} // namespace wzd
#endif
