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

const CLSID CLSID_NULL = GUID_NULL;

class Interface;
typedef void* OBJ_CREATE_FUNC(IID iid,
    Interface* pOuter,
    void* pParam);
typedef unsigned int OBJ_VERSION;

#define MAKE_VERSION(a,b,c,d) \
( ((OBJ_VERSION)d) | ( ((OBJ_VERSION)c) << 8 ) | ( ((OBJ_VERSION)b) << 16 ) | ( ((OBJ_VERSION)a) << 24 ) )

struct ClassInfo
{
    const CLSID* pClsid;
    const char* pName;
    const char* pType;
    OBJ_CREATE_FUNC* pFunc;
    OBJ_VERSION ver;
    void* pExt;
};
struct Interface
{
    STDMETHOD_(const ClassInfo&,Class)(
        ) PURE;
    STDMETHOD_(void*,Query)(
        IID iid
        ) PURE;
    STDMETHOD_(REFTYPE,AddRef)(
        ) PURE;
    STDMETHOD_(REFTYPE,Release)(
        ) PURE;
};

#endif // INTERFACE_H
