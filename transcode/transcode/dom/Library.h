#ifndef LIBRARY_H
#define LIBRARY_H
#include "Module.h"

using namespace std;

class CLibrary : public CDomSite
{
    struct class_info
    {
        const ClassInfo* pInfo;
        CDomSite* pSite;
    };

	typedef multimap<string,const class_info> Classes;
	typedef Classes::iterator ClassIt;
	typedef pair<Classes::key_type,Classes::mapped_type> ClassPair;

    friend class CModule;
    public:
        DOM_DECLARE(CLibrary)
        //ISite
        STDMETHODIMP Register(const ClassInfo& info);
        STDMETHODIMP_(const ClassInfo*) Enum(const ClassInfo* pPrev,const char* pType,const char* pName);
        STDMETHODIMP Load(const char* pLib);
        STDMETHODIMP_(void*) CreateObj(REFCLSID class_id,IID iface_id,Interface* pOuter,void* pParam);
        STDMETHODIMP DumpSetCallback(DUMP_CALLBACK_FUNC* pFunc,void* pTag);
        STDMETHODIMP DumpSetFile(const char* pPath,long szMax);
        STDMETHODIMP_(const char*) DumpGetFile();
        STDMETHODIMP DumpSetFlag(DUMP_FLAG flag);
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
        CModule::Collection m_modules;
        Classes m_classes;
        ClassIt m_it;
        DUMP_CALLBACK_FUNC* m_pFunc;
        void* m_pTag;
        string m_file;
        long m_szFile;
        FILE* m_pFile;
};

#endif // LIBRARY_H
