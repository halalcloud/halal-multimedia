#ifndef INTERFACE_H
#define INTERFACE_H
#include <stdint.h>
#include <typeinfo>
#include <string.h>

#define INTERFACE(iface)                struct iface : public Interface
#define INTERFACE_(iface, baseiface)    struct iface : public baseiface
#define HRESULT int32_t
#define S_FALSE 1
#define S_OK 0
#define IS_OK(hr) S_OK <= hr
#define IS_FAIL(hr) S_OK > hr
#define E_FAIL        -1
#define E_INVALIDARG  -2
#define E_OUTOFMEMORY -3
#define STDMETHODIMP             HRESULT
#define STDMETHODIMP_(type)      type
#define STDMETHOD(method)        virtual __attribute__((nothrow)) STDMETHODIMP method
#define STDMETHOD_(type,method)  virtual __attribute__((nothrow)) STDMETHODIMP_(type) method
#define PURE =0

typedef struct _GUID {
    unsigned int   Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;

// {17942BD2-218F-4fed-9AE2-B06CEE4E9B08}
static const GUID GUID_NULL =
{ 0x0, 0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };

typedef const char* IID;
typedef GUID  CLSID;
typedef const CLSID&  REFCLSID;
typedef uint32_t REFTYPE;
const IID IID_NULL = NULL;

#define IID(type) typeid(type*).name()
#define STR_CMP(str1,str2)  ((str1 == str2) || (IID_NULL != str1 && IID_NULL != str2 && 0 == strcmp(str1,str2)))
#define IID_CMP(iid1,iid2)  STR_CMP(iid1,iid2)
#define GUID_CMP(guid1,guid2) (guid1.Data1 == guid2.Data1 && guid1.Data2 == guid2.Data2 && guid1.Data3 == guid2.Data3 && 0 == memcmp(guid1.Data4,guid2.Data4,8))
#define CLSID_CMP GUID_CMP

class Interface;
typedef void* OBJ_CREATE_FUNC(IID iid,
    Interface* pOuter,
    bool aggregate,
    void* pParam);
typedef unsigned int OBJ_VERSION;

#define MAKE_VERSION(a,b,c,d) \
( ((OBJ_VERSION)d) | ( ((OBJ_VERSION)c) << 8 ) | ( ((OBJ_VERSION)b) << 16 ) | ( ((OBJ_VERSION)a) << 24 ) )

// {50926260-ADB4-4383-A7BF-87E3D9CC07E8}
static const CLSID CLSID_CEventPoint =
{ 0x50926260, 0xadb4, 0x4383, { 0xa7, 0xbf, 0x87, 0xe3, 0xd9, 0xcc, 0x7, 0xe8 } };


struct ClassInfo
{
    CLSID clsid;
    const char* name;
    const char* major;
    uint32_t    sub;
    const char* desc;
    OBJ_CREATE_FUNC* func;
    OBJ_VERSION version;
    void* ext1;
    void* ext2;
};

struct Interface
{
    STDMETHOD_(const ClassInfo&,Class)(
        ) PURE;
    STDMETHOD_(void*,Query)(
        IID iid,
        bool internal = false
        ) PURE;
    STDMETHOD_(REFTYPE,AddRef)(
        ) PURE;
    STDMETHOD_(REFTYPE,Release)(
        ) PURE;
    STDMETHOD_(Interface*,GetOwner)(
        ) PURE;
};

INTERFACE(IIt)
{
    STDMETHOD(Set)(
        void* it
        ) PURE;
    STDMETHOD(Get)(
        void* it
        ) PURE;
    STDMETHOD_(Interface*,Get)(
        ) PURE;
    STDMETHOD_(bool,Next)(
        ) PURE;
    STDMETHOD_(bool,Erase)(
        ) PURE;
};

INTERFACE(ICallback)
{
    STDMETHOD(OnEvent)(
        Interface* source,
        IIt* it,
        uint32_t type,
        int32_t param1 = 0,
        void* param2 = NULL,
        void* param3 = NULL
        ) PURE;
};

INTERFACE_(IEventCallback,ICallback)
{
    STDMETHOD_(void,Set)(
        Interface* obj,
        bool erase = true
        ) PURE;
};

INTERFACE(IEventPoint)
{
    STDMETHOD(NotifySet)(
        Interface* obj
        ) PURE;
    STDMETHOD_(IEventCallback*,NotifyGet)(
        ) PURE;
    STDMETHOD(Notify)(
        uint32_t type,
        int32_t param1 = 0,
        void* param2 = NULL,
        void* param3 = NULL
        ) PURE;
};

INTERFACE_(ISet,ICallback)
{
    STDMETHOD(CreateIt)(
        IIt** ppIt,
        void* pIt = NULL
        ) PURE;
    STDMETHOD_(bool,Next)(
        void* it
        ) PURE;
    STDMETHOD_(bool,Erase)(
        void* it
        ) PURE;
    STDMETHOD_(void,Clear)(
        ) PURE;
    STDMETHOD_(bool,Empty)(
        ) PURE;
};


#endif // INTERFACE_H
