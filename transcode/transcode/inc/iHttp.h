#ifndef IHTTP_INCLUDED
#define IHTTP_INCLUDED
#include "interface.h"

// {E94983DA-E465-4841-BBC6-3CD963E076D1}
static const CLSID CLSID_CHttpStream =
{ 0xe94983da, 0xe465, 0x4841, { 0xbb, 0xc6, 0x3c, 0xd9, 0x63, 0xe0, 0x76, 0xd1 } };

// {45F66636-84C7-4332-A92B-5ADE42F70D78}
static const CLSID CLSID_CHttpSession =
{ 0x45f66636, 0x84c7, 0x4332, { 0xa9, 0x2b, 0x5a, 0xde, 0x42, 0xf7, 0xd, 0x78 } };

INTERFACE(IHttpSession)
{

};
#endif // IHTTP_INCLUDED
