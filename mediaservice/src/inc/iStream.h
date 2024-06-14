#ifndef IMSG_H_INCLUDED
#define IMSG_H_INCLUDED

#include <stdint.h>
#include <stdarg.h>
#include <interface.h>
// {95250237-B43A-4653-A414-BA3FC26EEF87}
static const CLSID CLSID_CNetworkStream =
{ 0x95250237, 0xb43a, 0x4653, { 0xa4, 0x14, 0xba, 0x3f, 0xc2, 0x6e, 0xef, 0x87 } };

// {D35B0FA5-C61A-436D-BCEC-D18CA6068377}
static const CLSID CLSID_CNetworkListen =
{ 0xd35b0fa5, 0xc61a, 0x436d, { 0xbc, 0xec, 0xd1, 0x8c, 0xa6, 0x6, 0x83, 0x77 } };

// {3B653AD5-3277-4D6F-9073-5D46DE7525C3}
static const CLSID CLSID_CFileStream =
{ 0x3b653ad5, 0x3277, 0x4d6f, { 0x90, 0x73, 0x5d, 0x46, 0xde, 0x75, 0x25, 0xc3 } };

// {9D9E6856-3B12-47FB-B699-EB4064566D52}
static const CLSID CLSID_CEpoll =
{ 0x9d9e6856, 0x3b12, 0x47fb, { 0xb6, 0x99, 0xeb, 0x40, 0x64, 0x56, 0x6d, 0x52 } };

const int INVALID_FD = -1;

const char LISTEN_TYPE_NAME[] = "listen";
const char STREAM_TYPE_NAME[] = "stream";

const uint32_t ET_EOF             = 0;
const uint32_t ET_Error           = ET_EOF + 1;
const uint32_t ET_Epoll_Input     = ET_Error + 1;
const uint32_t ET_Epoll_Output    = ET_Epoll_Input + 1;
const uint32_t ET_Epoll_Timer     = ET_Epoll_Output + 1;
const uint32_t ET_Stream_Read     = ET_Epoll_Timer + 1;
const uint32_t ET_Stream_Write    = ET_Stream_Read + 1;
const uint32_t ET_Listen_Accept   = ET_Stream_Write + 1;
const uint32_t ET_Session_Push    = ET_Listen_Accept + 1;
const uint32_t ET_Session_Pull    = ET_Session_Push + 1;

struct ILocker
{
    STDMETHOD(Lock)(
        ) PURE;
    STDMETHOD(Unlock)(
        ) PURE;
    STDMETHOD_(bool,TryLock)(
        ) PURE;
};

INTERFACE(IEpollPoint)
{
    STDMETHOD_(ILocker*,GetLocker)(
        ) PURE;
    STDMETHOD(Add)(
        int fd
        ) PURE;
    STDMETHOD(Del)(
        int fd
        ) PURE;
    STDMETHOD_(uint32_t,GetCount)(
        ) PURE;
    STDMETHOD(SetTimer)(
        int32_t id,
        uint32_t duration,
        bool one_shoot = false
        ) PURE;
    STDMETHOD_(uint64_t,GetClock)(
        ) PURE;
    STDMETHOD_(void,SetTag)(
        void* tag
        ) PURE;
    STDMETHOD_(void*,GetTag)(
        ) PURE;
};

INTERFACE(IEpoll)
{
    STDMETHOD(CreatePoint)(
        ICallback* callback,
        IEpollPoint** ppEP
        ) PURE;
};

const HRESULT E_AGAIN  = -100;
const HRESULT E_EOF    = -101;
const HRESULT E_HANGUP = -102;

INTERFACE(IStream)
{
    enum seek_type
    {
        begin = 0,
        current,
        end
    };
    struct status
    {
        uint64_t read_real_size;
        uint64_t write_real_size;
        uint64_t read_total_size;
        uint64_t write_total_size;
    };

    static const uint32_t READ_FLAG  = 1;
    static const uint32_t WRITE_FLAG = 1<<1;
    static const uint32_t ANSY_FLAG  = 1<<2;
    static const uint32_t SEEK_FLAG  = 1<<3;
    static const uint32_t INTERACTION_FLAG  = READ_FLAG | WRITE_FLAG;

    static const uint32_t READ_FLAG_PEEK = 1;

    static const uint32_t WRITE_FLAG_REFFER    = 1;
    static const uint32_t WRITE_FLAG_FRAME     = WRITE_FLAG_REFFER<<1;
    static const uint32_t WRITE_FLAG_INTERFACE = WRITE_FLAG_FRAME<<1;
    static const uint32_t WRITE_FLAG_EOF       = WRITE_FLAG_INTERFACE<<1;

    STDMETHOD_(uint32_t,GetFlag)(
        ) PURE;
    STDMETHOD(Open)(
        const char* pUrl = NULL,
        uint32_t mode = 0
        ) PURE;
    STDMETHOD_(void,Close)(
        ) PURE;
    STDMETHOD(SetEventEnable)(
        bool enable
        ) PURE;
    STDMETHOD_(bool,GetEventEnable)(
        ) PURE;
    STDMETHOD(SetTimer)(
        int id,
        uint32_t duration,
        bool one_shoot = false
        ) PURE;
    STDMETHOD_(bool,CanRead)(
        ) PURE;
    STDMETHOD_(bool,CanWrite)(
        ) PURE;
    STDMETHOD(Read)(
        void* pBuf,
        uint32_t szBuf,
        uint8_t flag = 0,
        void** ppBuf = NULL
        ) PURE;
    STDMETHOD(Write)(
        const void* pBuf,
        uint32_t szBuf = 0,
        uint8_t flag = 0,
        void** ppBuf = NULL
        ) PURE;
    STDMETHOD(Flush)(
        ) PURE;
    STDMETHOD_(int64_t,GetPos)(
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
    STDMETHOD_(status&,GetStatus)(
        ) PURE;
};

INTERFACE(ISerialize)
{
    STDMETHOD(Load)(
        IStream* pStream,
        uint8_t flag = 0,
        void* param = NULL
        ) PURE;
    STDMETHOD(Save)(
        IStream* pStream,
        uint8_t flag = 0,
        void* param = NULL
        ) PURE;
};



#endif // IMSG_H_INCLUDED
