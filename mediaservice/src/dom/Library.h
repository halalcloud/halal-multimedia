#ifndef LIBRARY_H
#define LIBRARY_H

#include "Module.h"

using namespace std;

class CLibrary : public CDomSite
{
	typedef map< string,dom_ptr<ISite> > Modules;
	typedef Modules::iterator ModuleIt;
	typedef pair<Modules::key_type,Modules::mapped_type> ModulePair;

	typedef multimap<string,const class_info> Classes;
	typedef Classes::iterator ClassIt;
	typedef pair<Classes::key_type,Classes::mapped_type> ClassPair;

    friend class CModule;
    public:
        DOM_DECLARE(CLibrary)
        //ISite
        STDMETHODIMP Register(const ClassInfo& info);
        STDMETHODIMP EnumModule(IIt** ppIt);
        STDMETHODIMP_(ISite*) GetModule(IIt* pIt);
        STDMETHODIMP EnumClass(IIt** ppIt);
        STDMETHODIMP_(const class_info*) GetClass(IIt* pIt);
        STDMETHODIMP Load(const char* pLib);
        STDMETHODIMP_(void*) CreateObj(REFCLSID class_id,IID iface_id,Interface* pOuter,bool aggregate,void* pParam);
        STDMETHODIMP DumpSetCallback(DUMP_CALLBACK_FUNC* pFunc,void* pTag);
        STDMETHODIMP DumpSetFile(const char* pPath,long szMax);
        STDMETHODIMP_(const char*) DumpGetFile();
        STDMETHODIMP DumpSetFlag(DUMP_FLAG flag);
        STDMETHODIMP_(IProfile*) GetProfile();
        STDMETHODIMP_(void) SetObj(Interface* pObj);
        STDMETHODIMP_(Interface*) GetObj();
   protected:
        HRESULT InternalDump(const char* pModule,DUMP_FLAG flag,int min,int max,HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pContent);
        bool CreateDirectory(const string& path);
        FILE* FileOpen(const char* pTime,bool isAppend);
        bool FileOutput(const char* pTime,const char* pStr);
        void FileClose();
        HRESULT Register(CDomSite* pSite,const ClassInfo& info);
        bool LoadModule(const char* pLib);
    public:
        void Clear();
    protected:
        dom_ptr<SET(Modules)> m_modules;
        dom_ptr<SET(Classes)> m_classes;
        //Classes m_classes;
        ClassIt m_it;
        DUMP_CALLBACK_FUNC* m_pFunc;
        void* m_pTag;
        string m_file;
        long m_szFile;
        FILE* m_pFile;
        dom_ptr<IProfile> m_spProfile;
        dom_ptr<Interface> m_spObj;
};

#endif // LIBRARY_H
