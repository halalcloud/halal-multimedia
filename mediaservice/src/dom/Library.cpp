//#include <linux/module.h>
#include "Library.h"
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <interface_imp.h>
#include <dump.h>
#include "MemProfile.h"

string guid_to_string(const GUID& guid);
const unsigned int DUMP_FILE_MAX_SIZE_DEFAULT = 10*1024*1024;
const long DUMP_FILE_MIN_SIZE = 1*1024*1024;
const char DUMP_FILE_EXT_DEFAULT[] = ".log";
const char LOG_STR_FORMAT[] = "[Module]:%s\n[Type]:Log\t[time]:%s\t[Level]:%d\n[Description]:%s\n";
const char ERR_STR_FORMAT[] = "[Module]:%s\n[Type]:Error\t[time]:%s\t[Return]:%d\n[File]:%s\n[Line]:%d\t[Code]:%s\n[Description]:%s\n";

CLibrary::CLibrary()
:m_pFunc(NULL)
,m_pTag(NULL)
,m_szFile(DUMP_FILE_MAX_SIZE_DEFAULT)
,m_pFile(NULL)
{
    //ctor
}

bool CLibrary::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(NULL != (m_modules = SET(Modules)::Create()),false);
    JCHK(NULL != (m_classes = SET(Classes)::Create()),false);

    m_it = m_classes->m_set.end();

    JCHK(m_spProfile.p = (IProfile*)CMemProfile::Create(IID(IProfile),this,false,NULL),false);
    char path[PATH_MAX] = {0};
    if(0 >= readlink("/proc/self/exe", path,PATH_MAX))
        return false;
    m_path = path;
    m_file = path;
    m_file.append("_dump/");
    return true;
}

bool CLibrary::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear();
        if(NULL != g_pSite)
            g_pSite = NULL;
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CLibrary)
DOM_QUERY_IMPLEMENT_END_BASE(CDomSite)

STDMETHODIMP CLibrary::Register(const ClassInfo& info)
{
    return Register(this,info);
}

STDMETHODIMP CLibrary::EnumModule(IIt** ppIt)
{
    return m_modules->CreateIt(ppIt);
}

STDMETHODIMP_(ISite*) CLibrary::GetModule(IIt* pIt)
{
    JCHK(NULL != pIt,NULL);
    ModuleIt it;
    pIt->Get(&it);
    return it->second;
}

STDMETHODIMP CLibrary::EnumClass(IIt** ppIt)
{
    return m_classes->CreateIt(ppIt);
}

STDMETHODIMP_(const class_info*) CLibrary::GetClass(IIt* pIt)
{
    JCHK(NULL != pIt,NULL);
    ClassIt it;
    pIt->Get(&it);
    return &it->second;
}

STDMETHODIMP CLibrary::Load(const char* pLib)
{
    JCHK(NULL != pLib,E_FAIL);
    if('/' == pLib[strlen(pLib)-1])
    {
        if('/' != pLib[0])
        {
            string dir = m_path;
            size_t pos = dir.find_last_of('/');
            JCHK1(string::npos != pos,E_FAIL,"load module[%s] fail",m_path.c_str());
            dir.erase(pos+1);
            JCHK1(-1 != chdir(dir.c_str()),E_FAIL,"load module can not cd dir[%s]",dir.c_str());
        }

        DIR *dp;
        JCHK1(NULL != (dp = opendir(pLib)),E_FAIL,"load module can not open dir[%s]",pLib);

        struct dirent *entry;
        //struct stat statbuf;
        while(NULL != (entry = readdir(dp)))
        {
            char* pDot = strrchr(entry->d_name,'.');
            if(NULL != pDot && 0 != strcmp(entry->d_name,"libdom.so") && 0 == strcmp(pDot,".so"))
            {
                string path = pLib;
                path.append(entry->d_name);
                LoadModule(path.c_str());
            }
        }
        closedir(dp);
        return S_OK;
    }
    else
        return true == LoadModule(pLib) ? S_OK : E_FAIL;
}

STDMETHODIMP_(void*) CLibrary::CreateObj(REFCLSID class_id,IID iface_id,Interface* pOuter,bool aggregate,void* pParam)
{
    string str = guid_to_string(class_id);
    ClassIt it = m_classes->m_set.find(str);
    if(m_classes->m_set.end() != it)
        return it->second.pInfo->func(iface_id,pOuter,aggregate,pParam);
    else
        return NULL;
}

STDMETHODIMP CLibrary::DumpSetCallback(DUMP_CALLBACK_FUNC* pFunc,void* pTag)
{
    m_pFunc = pFunc;
    m_pTag = pTag;
    return S_OK;
}

STDMETHODIMP CLibrary::DumpSetFile(const char* pPath,long szMax)
{
    if(NULL != pPath)
    {
        FileClose();
        m_file = pPath;
    }
    if(szMax >= DUMP_FILE_MIN_SIZE)
        m_szFile = szMax;
    return S_OK;
}

STDMETHODIMP_(const char*) CLibrary::DumpGetFile()
{
    return m_file.c_str();
}

STDMETHODIMP CLibrary::DumpSetFlag(DUMP_FLAG flag)
{
    HRESULT hr;
    dom_ptr<IIt> spIt;
    JIF(m_modules->CreateIt(&spIt));
    while(spIt->Next())
    {
        dom_ptr<ISite> spSite;
        JCHK(spIt.Query(&spSite),E_FAIL);
        JIF(spSite->DumpSetFlag(flag));
    }
    JIF(CDomSite::DumpSetFlag(flag));
    return hr;
}

STDMETHODIMP_(IProfile*) CLibrary::GetProfile()
{
    return m_spProfile.p;
}

STDMETHODIMP_(void) CLibrary::SetObj(Interface* pObj)
{
    m_spObj = pObj;
}

STDMETHODIMP_(Interface*) CLibrary::GetObj()
{
    return m_spObj;
}

HRESULT CLibrary::InternalDump(const char* pModule,DUMP_FLAG flag,int min,int max,HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pContent)
{
	char strTime[20];
	time_t long_time = time(NULL);
	const tm* pTM = localtime(&long_time);
	snprintf(strTime,20,"%04d-%02d-%02d_%02d-%02d-%02d",
        pTM->tm_year+1900,pTM->tm_mon+1,pTM->tm_mday,pTM->tm_hour,pTM->tm_min,pTM->tm_sec);
    if(IS_OK(hr))
    {
        if(hr >= min && hr <= max)
        {
            char str[1024] = {0};
            snprintf(str,1024,LOG_STR_FORMAT,pModule,strTime,hr,pContent);
            if(0 != (DOT_LOG_TRACE & flag))
            {

            }
            if(0 != (DOT_LOG_FILE & flag))
            {
                FileOutput(strTime,str);
            }
            if(0 != (DOT_LOG_PRINTF & flag))
            {
                printf("%s",str);
            }
        }
    }
    else
    {

        char str[1024] = {0};
        snprintf(str,1024,ERR_STR_FORMAT,pModule,strTime,hr,pFile,line,pCode,pContent);
        if(0 != (DOT_ERR_TRACE & flag))
        {
        }
        if(0 != (DOT_ERR_MSGBOX & flag))
        {
        }
        if(0 != (DOT_ERR_FILE & flag))
        {
            FileOutput(strTime,str);
        }
        if(0 != (DOT_ERR_PRINTF & flag))
        {
            printf("%s",str);
        }
        if(0 != (DOT_ERR_EXCEPT & flag))
        {

        }
        if(0 != (DOT_ERR_ASSERT & flag))
        {
            assert(false);
        }
    }

    if(NULL != m_pFunc)
        m_pFunc(hr,pModule,pContent,m_pTag);
    return hr;
}

bool CLibrary::CreateDirectory(const string& path)
{
	size_t slash;
    string directory;
	if(string::npos == (slash = path.find_first_of('/')))
		return false;
	while(string::npos != (slash = path.find_first_of('/',slash + 1)))
	{
		directory.assign(path,0,slash+1);
        if(0 != access(directory.c_str(),R_OK))
        {
            if(mkdir(directory.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
                return false;
        }
	}
	return true;
}

FILE* CLibrary::FileOpen(const char* pTime,bool isAppend)
{
	if(NULL != m_pFile)
		return m_pFile;

    string path = m_file;
    if(path.at(path.length()-1) == '/')
    {
        path += pTime;
        path += DUMP_FILE_EXT_DEFAULT;
    }
    if(false == CreateDirectory(path))
        return NULL;
    m_pFile = fopen(path.c_str(),true == isAppend ? "a" : "w");
    return m_pFile;
}

bool CLibrary::FileOutput(const char* pTime,const char* pStr)
{
    FILE* pFile = FileOpen(pTime,true);
    if(NULL == pFile)
        return false;
    long len = ftell(pFile);
	if(len > m_szFile)
	{
        FileClose();
        pFile = FileOpen(pTime,false);
        if(NULL == pFile)
            return false;
	}
	fwrite(pStr,sizeof(char),strlen(pStr),pFile);
	fflush(pFile);
    return true;
}

void CLibrary::FileClose()
{
    if(NULL != m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

HRESULT CLibrary::Register(CDomSite* pSite,const ClassInfo& info)
{
    JCHK(NULL != pSite,E_INVALIDARG);
    //JCHK(NULL != info.clsid,E_INVALIDARG);
    JCHK(NULL != info.func,E_INVALIDARG);

    ClassIt it;
    string guid = guid_to_string(info.clsid);

    class_info ci = {&info,pSite};

	pair<ClassIt,ClassIt> ret = m_classes->m_set.equal_range(guid);
	for(it = ret.first ; it != ret.second ; ++it)
	{
		if(info.version > it->second.pInfo->version)
		{
			m_classes->m_set.insert(it,ClassPair(guid,ci));
			return S_OK;
		}
	}
	m_classes->m_set.insert(it,ClassPair(guid,ci));
	return S_OK;
}

bool CLibrary::LoadModule(const char* pLib)
{
    ModuleIt it;

    if(NULL == pLib)
        return NULL;

    if(m_modules->m_set.end() != (it = m_modules->m_set.find(pLib)))
    {
        LOG(0,"load module[%s] already loaded",pLib);
        return true;
    }
    dom_ptr<ISite> spModule;
    JCHK(spModule.p = static_cast<ISite*>(CModule::Create(IID(ISite),this,false,(void*)pLib)),false);
    pair<ModuleIt,bool> result = m_modules->m_set.insert(ModulePair(pLib,spModule));
    JCHK(true == result.second,false);
    LOG(0,"load module[%s] OK",pLib);
    return true;
}

void CLibrary::Clear()
{
    m_classes->Clear();
    m_modules->Clear();
}
