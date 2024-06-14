#ifndef MODULE_H
#define MODULE_H
#include <map>
#include "DomSite.h"

class CLibrary;
class CModule : public CDomSite
{
    friend class CLibrary;
    public:
        DOM_DECLARE(CModule)
        //ISite
        STDMETHODIMP Register(const ClassInfo& info);
        STDMETHODIMP EnumModule(IIt** ppIt);
        STDMETHODIMP_(ISite*) GetModule(IIt* pIt);
        STDMETHODIMP EnumClass(IIt** ppIt);
        STDMETHODIMP_(const class_info*) GetClass(IIt* pIt);
        STDMETHODIMP Load(const char* pLib);
        STDMETHODIMP_(void*)CreateObj(REFCLSID class_id,IID iface_id,Interface* pOuter,bool aggregate,void* pParam);
        STDMETHODIMP DumpSetCallback(DUMP_CALLBACK_FUNC* pFunc,void* pTag);
        STDMETHODIMP DumpSetFile(const char* pPath,long szMax);
        STDMETHODIMP_(const char*) DumpGetFile();
        STDMETHODIMP_(IProfile*) GetProfile();
        STDMETHODIMP_(void) SetObj(Interface* pObj);
        STDMETHODIMP_(Interface*) GetObj();
    protected:
        //CModule
        HRESULT InternalDump(const char* pModule,DUMP_FLAG flag,int min,int max,HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pContent);
        bool InternalLoad(const char* pLib);
    protected:
        CLibrary* m_pLibrary;
        void* m_handle;
        DOM_SET_SITE_FUNC* m_pFuncSet;
};

//class TIt : public IIterator
//{
//public:
//    TIt(TSet* pSet):m_pSet(pSet){}
//    ~TIt(){}
//        STDMETHODIMP_(void) Reset()
//        {
//            m_it = m_pSet->m_set.end();
//        }
//        STDMETHODIMP_(bool) Next()
//        {
//            return ++m_it != m_pSet->m_set.end();
//        }
//        STDMETHODIMP_(bool) Erase()
//        {
//            if(m_it == m_pSet->m_set.end())
//                return true;
//            m_it = m_pSet->m_set.erase(m_it);
//            return it == m_pSet->m_set.end();
//        }
//        STDMETHODIMP_(bool) Find(...)
//        {
//
//        }
//        STDMETHODIMP_(const ClassInfo&) Class()
//        {
//
//        }
//        STDMETHODIMP_(void*) Query(IID iid,bool internal = false)
//        {
//
//        }
//        STDMETHODIMP_(REFTYPE) AddRef()
//        {
//
//        }
//        STDMETHODIMP_(REFTYPE) Release()
//        {
//
//        }
//    protected:
//        REFTYPE m_ref;
//        TSet* m_pSet;
//        SetIt m_it;
//    };
//};
#endif // MODULE_H
