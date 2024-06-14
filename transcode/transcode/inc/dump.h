#ifndef DUMP_H_INCLUDED
#define DUMP_H_INCLUDED
#include "dom_site.h"
enum DUMP_OUTPUT_TYPE
{
	DOT_ERR_ASSERT  = 0x0001, // Assert.
	DOT_ERR_TRACE   = 0x0002, // Display information in debug model(it use TRACE(...)).
	DOT_ERR_MSGBOX  = 0x0004, // Popup a message box to show the info.
	DOT_ERR_FILE    = 0x0008, // Dump the message to the file.
	DOT_ERR_PRINTF  = 0x0010, // Dump message to console.
	DOT_ERR_EXCEPT  = 0x0020, // Throw an exception.
	DOT_ERR_MASK    = 0x00FF,

	DOT_LOG_TRACE   = 0x0100, // Trace
	DOT_LOG_FILE    = 0x0200, // Trace to file
	DOT_LOG_PRINTF  = 0x0400, // Trace to console.
	DOT_LOG_MASK    = 0xFF00
};
//Trace(m,p[n]...)
//日志记录
//m:信息
//p[n]:参数
#define LOG g_pSite->Trace
#define LOG0(l,m)
#define LOG1(l,m, p1)
#define LOG2(l,m, p1, p2)
#define LOG3(l,m, p1, p2, p3)
#define LOG4(l,m, p1, p2, p3, p4)
#define LOG5(l,m, p1, p2, p3, p4, p5)
#define LOG6(l,m, p1, p2, p3, p4, p5, p6)
#define LOG7(l,m, p1, p2, p3, p4, p5, p6, p7)
#define LOG8(l,m, p1, p2, p3, p4, p5, p6, p7, p8)

#define LCHK(x) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x);\

#define LCHK0(x,m) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m);\

#define LCHK1(x,m,p1) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1);\

#define LCHK2(x,m,p1,p2) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2);\

#define LCHK3(x,m,p1,p2,p3) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3);\

#define LCHK4(x,m,p1,p2,p3,p4) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3,p4);\

#define LCHK5(x,m,p1,p2,p3,p4,p5) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3,p4,p5);\

#define LCHK6(x,m,p1,p2,p3,p4,p5,p6) \
    if(!(x)) g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3,p4,p5,p6);\

#define JUMP(x,r) \
    if(!(x)) return r; \

#define JCHK(x,r) \
    if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x); return r;}\

#define JCHK0(x,r,m) \
    if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m); return r;}\

#define JCHK1(x,r,m,p1) \
    if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1); return r;}\

#define JCHK2(x,r,m,p1,p2) \
    if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2); return r;}\

#define JCHK3(x,r,m,p1,p2,p3) \
    if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3); return r;}\

#define JCHK4(x,r,m,p1,p2,p3,p4) \
     if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3,p4); return r;}\

#define JCHK5(x,r,m,p1,p2,p3,p4,p5) \
     if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3,p4,p5); return r;}\

#define JCHK6(x,r,m,p1,p2,p3,p4,p5,p6) \
     if(!(x)) { g_pSite->Check(E_FAIL,__FILE__,__LINE__,#x,m,p1,p2,p3,p4,p5,p6); return r;}\

#define JIF(x) \
	if(0 > (hr = (x))) return hr; \


#endif // DUMP_H_INCLUDED
