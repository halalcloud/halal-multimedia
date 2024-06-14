#ifndef DOMSITE_H
#define DOMSITE_H
#include <string>
#include <dom.h>
using namespace std;

class CDomSite : public ISite
{
    public:
        CDomSite();
        virtual ~CDomSite();
        //Interface
        STDMETHODIMP_(void*) Query(IID iid);
        //ISite
        STDMETHODIMP_(const char*) GetPath();
        STDMETHODIMP DumpSetFlag(DUMP_FLAG flag);
        STDMETHODIMP_(DUMP_FLAG) DumpGetFlag();
        STDMETHODIMP DumpSetLogLevelRange(int min,int max);
        STDMETHODIMP DumpGetLogLevelRange(int* pMin,int* pMax);
        STDMETHODIMP Trace(int level,const char* pFormat,...);
        STDMETHODIMP Check(HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pFormat,...);
    protected:
        virtual HRESULT InternalDump(const char* pModule,DUMP_FLAG flag,int min,int max,HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pContent) = 0;
    protected:
        string m_path;
        DUMP_FLAG m_flag;
        int m_min;
        int m_max;
};

#endif // DOMSITE_H
