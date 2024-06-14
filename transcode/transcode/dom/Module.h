#ifndef MODULE_H
#define MODULE_H
#include <map>
#include "DomSite.h"

class CLibrary;
class CModule : public CDomSite
{
    friend class CLibrary;
	typedef map< string,dom_ptr<ISite> > Collection;
	typedef Collection::iterator It;
	typedef pair<Collection::key_type,Collection::mapped_type> Pair;
    public:
        DOM_DECLARE(CModule)
        //ISite
        STDMETHODIMP Register(const ClassInfo& info);
        STDMETHODIMP_(const ClassInfo*) Enum(const ClassInfo* pPrev,const char* pType,const char* pName);
        STDMETHODIMP Load(const char* pLib);
        STDMETHODIMP_(void*)CreateObj(REFCLSID class_id,IID iface_id,Interface* pOuter,void* pParam);
        STDMETHODIMP DumpSetCallback(DUMP_CALLBACK_FUNC* pFunc,void* pTag);
        STDMETHODIMP DumpSetFile(const char* pPath,long szMax);
        STDMETHODIMP_(const char*) DumpGetFile();
    protected:
        //CModule
        HRESULT InternalDump(const char* pModule,DUMP_FLAG flag,int min,int max,HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pContent);
        bool InternalLoad(const char* pLib);
    protected:
        CLibrary* m_pLibrary;
        void* m_handle;
        DOM_SET_SITE_FUNC* m_pFuncSet;
};

#endif // MODULE_H
