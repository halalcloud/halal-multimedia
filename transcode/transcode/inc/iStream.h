#ifndef IMSG_H_INCLUDED
#define IMSG_H_INCLUDED

#include <stdint.h>
#include "interface.h"
#include "url.h"
// {95250237-B43A-4653-A414-BA3FC26EEF87}
static const CLSID CLSID_CNetworkStream =
{ 0x95250237, 0xb43a, 0x4653, { 0xa4, 0x14, 0xba, 0x3f, 0xc2, 0x6e, 0xef, 0x87 } };

// {D35B0FA5-C61A-436D-BCEC-D18CA6068377}
static const CLSID CLSID_CNetworkServer =
{ 0xd35b0fa5, 0xc61a, 0x436d, { 0xbc, 0xec, 0xd1, 0x8c, 0xa6, 0x6, 0x83, 0x77 } };

// {3B653AD5-3277-4D6F-9073-5D46DE7525C3}
static const CLSID CLSID_CFileStream =
{ 0x3b653ad5, 0x3277, 0x4d6f, { 0x90, 0x73, 0x5d, 0x46, 0xde, 0x75, 0x25, 0xc3 } };

// {9D9E6856-3B12-47FB-B699-EB4064566D52}
static const CLSID CLSID_CEpoll =
{ 0x9d9e6856, 0x3b12, 0x47fb, { 0xb6, 0x99, 0xeb, 0x40, 0x64, 0x56, 0x6d, 0x52 } };

// {C7F85B3A-18C6-4524-B84E-94911C5D8208}
static const CLSID CLSID_CBufSession =
{ 0xc7f85b3a, 0x18c6, 0x4524, { 0xb8, 0x4e, 0x94, 0x91, 0x1c, 0x5d, 0x82, 0x8 } };

// {50926260-ADB4-4383-A7BF-87E3D9CC07E8}
static const CLSID CLSID_CBuffer =
{ 0x50926260, 0xadb4, 0x4383, { 0xa7, 0xbf, 0x87, 0xe3, 0xd9, 0xcc, 0x7, 0xe8 } };

const char STREAM_TYPE_NAME[] = "Stream";

enum event_type
{
    et_timeout = 0,
    et_error,
    et_nb
};
class IEpoll;
INTERFACE(IEpollCallback)
{
    STDMETHOD(OnEvent)(
        uint32_t id,
        void* pParam = NULL
        ) PURE;
    STDMETHOD_(IEpoll*,GetEpoll)(
        ) PURE;

};

const int EPOLL_DEFAULT_TIME_OUT = -1;
INTERFACE(IEpoll)
{
    STDMETHOD(SetTimeout)(
        IEpollCallback* pCallback,
        int ms = EPOLL_DEFAULT_TIME_OUT
        ) PURE;
    STDMETHOD(Add)(
        int fd,
        IEpollCallback* pCallback
        ) PURE;
    STDMETHOD(Del)(
        int fd,
        IEpollCallback* pCallback
        ) PURE;
};

const static int E_AGAIN = -100;
const static int E_EOF   = -101;

INTERFACE(IStream)
{
    enum event
    {
        read = et_nb,
        write
    };
    enum seek_type
    {
        begin = 0,
        current,
        end
    };
    const static uint32_t ANSY_FLAG  = 0x00000001;
    const static uint32_t BOUND_FLAG = 0x00000002;
    const static uint32_t SEEK_FLAG  = 0x00000004;
    STDMETHOD_(uint32_t,GetFlag)(
        ) PURE;
    STDMETHOD(Open)(
        url* pUrl = NULL,
        int mode = 0
        ) PURE;
    STDMETHOD_(void,Close)(
        ) PURE;
    STDMETHOD(Read)(
        void* pBuf,
        int32_t szBuf
        ) PURE;
    STDMETHOD(Write)(
        void* pBuf,
        int32_t szBuf
        ) PURE;
    STDMETHOD_(int64_t,Seek)(
        int64_t position,
        seek_type type = begin
        ) PURE;
    STDMETHOD_(int64_t,GetLength)(
        ) PURE;
    STDMETHOD(SetLength)(
        int64_t len
        ) PURE;
    STDMETHOD_(bool,IsOpen)(
        ) PURE;
};

INTERFACE(ISerialize)
{
    STDMETHOD(Load)(
        IStream* pStream
        ) PURE;
    STDMETHOD(Save)(
        IStream* pStream
        ) PURE;
};

INTERFACE(IStreamServer)
{
    enum event
    {
        accept = et_nb
    };
    STDMETHOD(Startup)(
        url* pUrl
        ) PURE;
    STDMETHOD_(void,Shutdown)(
        ) PURE;
    STDMETHOD_(uint16_t,GetPort)(
        ) PURE;
    STDMETHOD(Accept)(
        IStream** ppStream,
        IEpollCallback* pCallback = NULL
        ) PURE;
};

INTERFACE(IBuffer)
{
    uint8_t* data;
    uint32_t size;
    STDMETHOD(Alloc)(
        uint32_t size
        ) PURE;
    STDMETHOD_(uint32_t,GetSize)(
        ) PURE;
    STDMETHOD_(void,Clear)(
        ) PURE;
    STDMETHOD(CopyFrom)(
        IBuffer* pBuf
        ) PURE;
    STDMETHOD(CopyTo)(
        IBuffer* pBuf
        ) PURE;
};

INTERFACE(IBufSession)
{
    enum event
    {
        receive = et_nb,
        deliver
    };
    STDMETHOD(Open)(
        IStreamServer* pServer
        ) PURE;
    STDMETHOD(Open)(
        url* pUrl,
        int mode = 0
        ) PURE;
    STDMETHOD(Receive)(
        IBuffer** ppBuf
        ) PURE;
    STDMETHOD(Deliver)(
        IBuffer* pBuf
        ) PURE;
    STDMETHOD_(bool,IsOpen)(
        ) PURE;
};
#endif // IMSG_H_INCLUDED
