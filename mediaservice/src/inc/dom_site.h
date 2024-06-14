#ifndef DOM_SITE_H_INCLUDED
#define DOM_SITE_H_INCLUDED

#include "iProfile.h"

typedef HRESULT DUMP_CALLBACK_FUNC(HRESULT hr,   //hr<0:Error;hr>=0:LOG,hr=LOG level
    const char* pModule,                //modeule path
    const char* pContent,               //description
    void* pTag) ;
                           //tag
const int DUMP_LOG_MIN_LEVEL = 0;
const int DUMP_LOG_MAX_LEVEL = 0x7FFFFFFF;

class ISite;
struct class_info
{
    const ClassInfo* pInfo;
    ISite* pSite;
};

INTERFACE(ISite)
{
    typedef unsigned short DUMP_FLAG;
    STDMETHOD_(const char*,GetPath)(
        )PURE;
    STDMETHOD(Register)(
        const ClassInfo& info
        )PURE;
    STDMETHOD(EnumModule)(
        IIt** ppIt
        )PURE;
    STDMETHOD_(ISite*,GetModule)(
        IIt* pIt
        )PURE;
    STDMETHOD(EnumClass)(
        IIt** ppIt
        )PURE;
    STDMETHOD_(const class_info*,GetClass)(
        IIt* pIt
        )PURE;
    STDMETHOD(Load)(
        const char* pLib          //library path
        )PURE;
    STDMETHOD_(void*,CreateObj)(
        REFCLSID class_id,        //class name
        IID iface_id,             //interface name
        Interface* pOuter = NULL,      //Outer
        bool aggregate = false,   //aggregate
        void* pParam = NULL       //param
        )PURE;
    STDMETHOD(DumpSetCallback)(
        DUMP_CALLBACK_FUNC* pFunc,
        void* pTag
        )PURE;
    STDMETHOD(DumpSetFile)(
        const char* pPath,
        long szMax = 0
        )PURE;
    STDMETHOD_(const char*,DumpGetFile)(
        )PURE;
    STDMETHOD(DumpSetFlag)(
        DUMP_FLAG flag
        )PURE;
    STDMETHOD_(DUMP_FLAG,DumpGetFlag)(
        )PURE;
    STDMETHOD(DumpSetLogLevelRange)(
        int min = DUMP_LOG_MIN_LEVEL,
        int max = DUMP_LOG_MAX_LEVEL
        )PURE;
	STDMETHOD(Trace)(
		int level,                        //返回值
		const char* pFormat               //描述信息
		...) PURE;
	STDMETHOD(Check)(
		HRESULT hr,                       //返回值
		const char* pFile,                //代码所在文件
		unsigned int line,                //代码行
		const char* pCode,                //代码
		const char* pFormat = NULL,       //描述信息
		...) PURE;
    STDMETHOD_(IProfile*,GetProfile)(
            ) PURE;
    STDMETHOD_(void,SetObj)(
        Interface* pObj
            ) PURE;
    STDMETHOD_(Interface*,GetObj)(
            ) PURE;
};
extern ISite* g_pSite;

#ifdef DOM_EXPORTS
#define DOM_API extern "C" __attribute__ ((visibility("default")))
#else
#define DOM_API extern "C" __attribute__ ((visibility("default")))
#endif

DOM_API bool SetSite(ISite* pSite);
typedef bool DOM_SET_SITE_FUNC(ISite* pSite);
const char DOM_SET_SITE_FUNC_NAME[] = "SetSite";
DOM_API ISite* GetSite();

#define DOM_DECLARE(cls) \
    cls(); \
    virtual ~cls(){} \
    bool FinalConstruct(Interface* pOuter,void* pParam); \
    bool FinalDestructor(bool is_active); \
    static __attribute__((nothrow)) void* Create(IID iid = IID_NULL,Interface* pOuter = NULL,bool aggregate = false,void* pParam = NULL); \
    STDMETHOD_(const ClassInfo&,Class)() PURE; \
    STDMETHODIMP_(void*)Query(IID iid,bool internal); \
    STDMETHOD_(REFTYPE,AddRef)() PURE; \
    STDMETHOD_(REFTYPE,Release)() PURE; \

#define DOM_QUERY_IMPLEMENT_BEG(cls) \
void* cls::Create(IID iid,Interface* pOuter,bool aggregate,void* pParam) \
{ \
    bool br; \
    void* pResult = NULL; \
    interface_imp<cls> *pObj; \
    JCHK(pObj = new interface_imp<cls>(pOuter,aggregate,pParam,br),NULL); \
    if(true == br) \
    { \
        pResult = pObj->QueryInterface(iid); \
        if(NULL == pResult) \
            pObj->Release(); \
    } \
    return pResult; \
} \
STDMETHODIMP_(void*) cls::Query(IID iid,bool internal) \
{ \

#define DOM_QUERY_IMPLEMENT(__impInterface) \
    if(true == IID_CMP(iid,IID(__impInterface))) \
        return dynamic_cast<__impInterface*>(this);

#define DOM_QUERY_IMPLEMENT_AGGREGATE(__impIUnknown) \
{ \
    void* pResult = NULL; \
    if(__impIUnknown != NULL && NULL != (pResult =__impIUnknown->Query(iid,internal))) \
		return pResult; \
} \

#define DOM_QUERY_IMPLEMENT_END \
    return NULL; \
} \

#define DOM_QUERY_IMPLEMENT_END_BASE(cls) \
    return cls::Query(iid); \
} \

#define DOM_CLASS_EXPORT_BEG \
ISite* g_pSite; \
bool SetSite(ISite* pSite) \
{ \
    if(NULL != (g_pSite = pSite)) \
    { \

#define DOM_CLASS_EXPORT(class,maj,sb,dsc,ver,et1,et2) \
        { \
            interface_imp<class>::s_info.clsid = CLSID_##class; \
            interface_imp<class>::s_info.name = #class; \
            interface_imp<class>::s_info.major = maj; \
            interface_imp<class>::s_info.sub = sb; \
            interface_imp<class>::s_info.desc = dsc; \
            interface_imp<class>::s_info.func = class::Create; \
            interface_imp<class>::s_info.version = ver; \
            interface_imp<class>::s_info.ext1 = et1; \
            interface_imp<class>::s_info.ext2 = et2; \
            pSite->Register(interface_imp<class>::s_info); \
        } \

#define DOM_CLASS_EXPORT_END \
    } \
    return true; \
} \

#endif // DOM_SITE_H_INCLUDED
