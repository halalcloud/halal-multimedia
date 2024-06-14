#include "Module.h"
#include <dlfcn.h>
#include <interface_imp.h>
#include "Library.h"

CModule::CModule()
:m_pLibrary(NULL)
,m_handle(NULL)
,m_pFuncSet(NULL)
{
    //ctor
}

bool CModule::FinalConstruct(Interface* pOuter,void* pParam)
{
    if(NULL != pOuter)
    {
        m_pLibrary = static_cast<CLibrary*>(pOuter);
        m_flag = m_pLibrary->DumpGetFlag();
        m_pLibrary->DumpGetLogLevelRange(&m_min,&m_max);
    }
    return InternalLoad(static_cast<char*>(pParam));
}

bool CModule::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        if(NULL != m_pFuncSet)
            m_pFuncSet(NULL);
        if(NULL != m_handle)
            dlclose(m_handle);
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CModule)
DOM_QUERY_IMPLEMENT_END_BASE(CDomSite)

STDMETHODIMP CModule::Register(const ClassInfo& info)
{
    return m_pLibrary->Register(this,info);
}

STDMETHODIMP CModule::EnumModule(IIt** ppIt)
{
    return m_pLibrary->EnumModule(ppIt);
}

STDMETHODIMP_(ISite*) CModule::GetModule(IIt* pIt)
{
    return m_pLibrary->GetModule(pIt);
}

STDMETHODIMP  CModule::EnumClass(IIt** ppIt)
{
    return m_pLibrary->EnumClass(ppIt);
}

STDMETHODIMP_(const class_info*) CModule::GetClass(IIt* pIt)
{
    return m_pLibrary->GetClass(pIt);
}

STDMETHODIMP CModule::Load(const char* pLib)
{
    return m_pLibrary->Load(pLib);
}

STDMETHODIMP_(void*) CModule::CreateObj(REFCLSID class_id,IID iface_id,Interface* pOuter,bool aggregate,void* pParam)
{
    return m_pLibrary->CreateObj(class_id,iface_id,pOuter,aggregate,pParam);
}

STDMETHODIMP CModule::DumpSetCallback(DUMP_CALLBACK_FUNC* pFunc,void* pTag)
{
    return m_pLibrary->DumpSetCallback(pFunc,pTag);
}

STDMETHODIMP CModule::DumpSetFile(const char* pPath,long szMax)
{
    return E_FAIL;
}

STDMETHODIMP_(const char*) CModule::DumpGetFile()
{
    return m_pLibrary->DumpGetFile();
}

STDMETHODIMP_(IProfile*) CModule::GetProfile()
{
    return m_pLibrary->GetProfile();
}

STDMETHODIMP_(void) CModule::SetObj(Interface* pObj)
{
    m_pLibrary->SetObj(pObj);
}

STDMETHODIMP_(Interface*) CModule::GetObj()
{
    return m_pLibrary->GetObj();
}

HRESULT CModule::InternalDump(const char* pModule,DUMP_FLAG flag,int min,int max,HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pContent)
{
    return m_pLibrary->InternalDump(pModule,flag,min,max,hr,pFile,line,pCode,pContent);
}

bool CModule::InternalLoad(const char* pLib)
{
    if(NULL != dlopen(pLib,RTLD_NOLOAD))
    {
        LOG(0,"load module[%s] already load",pLib);
        return false;
    }

    m_handle = dlopen(pLib,RTLD_NOW);
    if(NULL == m_handle)
    {
        LOG(0,"load module[%s] fail,msg:%s",pLib,dlerror());
        return false;
    }

    DOM_SET_SITE_FUNC *pFuncSetSite = (DOM_SET_SITE_FUNC*)dlsym(m_handle,DOM_SET_SITE_FUNC_NAME);
    if(NULL == pFuncSetSite)
    {
        LOG(0,"load module[%s] is invalid",pLib);
        return false;
    }

    if(false == pFuncSetSite(this))
    {
        LOG(0,"load module[%s] initialize fail",pLib);
        return false;
    }

    m_pFuncSet = pFuncSetSite;
    m_path = pLib;
    return true;
}
