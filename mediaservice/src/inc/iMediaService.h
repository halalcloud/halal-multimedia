#ifndef IMEDIASERVICE_H_INCLUDED
#define IMEDIASERVICE_H_INCLUDED

#include <interface.h>
// {36783134-6105-4BF6-A72E-C7D2A4D7A95B}
static const CLSID CLSID_CMediaService =
{ 0x36783134, 0x6105, 0x4bf6, { 0xa7, 0x2e, 0xc7, 0xd2, 0xa4, 0xd7, 0xa9, 0x5b } };

enum method_type
{
    method_push = 0,
    method_pull,
    method_nb
};

const char METHOD_NAME[method_nb][5] = {"push","pull"};

enum target_type
{
    target_live = 0,
    target_file,
    target_vod,
    target_nb
};

const char TARGET_NAME[target_nb][5] = {"live","file","vod"};

INTERFACE(IMediaService)
{
    STDMETHOD(StartUp)(
        const char* pConfig
        ) PURE;
    STDMETHOD(Shutdown)(
        ) PURE;

};
#endif // IMEDIASERVICE_H_INCLUDED
