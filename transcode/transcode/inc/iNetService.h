#ifndef INETSERVICE_H_INCLUDED
#define INETSERVICE_H_INCLUDED
#include "interface.h"
// {44A3AFE9-2365-48E9-8C95-5E7F06D09E48}
static const CLSID CLSID_CHLSService =
{ 0x44a3afe9, 0x2365, 0x48e9, { 0x8c, 0x95, 0x5e, 0x7f, 0x6, 0xd0, 0x9e, 0x48 } };

INTERFACE(INetService)
{
    STDMETHOD(Start)(
        uint16_t port = 0
        )PURE;
    STDMETHOD(Stop)(
        )PURE;
};

#endif // INETSERVICE_H_INCLUDED

