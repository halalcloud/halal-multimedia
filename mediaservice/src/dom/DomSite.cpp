#include "DomSite.h"
#include <stdio.h>
#include <stdarg.h>
#include <dump.h>

CDomSite::CDomSite()
:m_flag(0)
,m_min(DUMP_LOG_MIN_LEVEL)
,m_max(DUMP_LOG_MAX_LEVEL)
{
#ifdef _DEBUG
    //m_flag = DOT_ERR_ASSERT|DOT_ERR_PRINTF|DOT_ERR_FILE|DOT_LOG_PRINTF|DOT_LOG_FILE;
    m_flag = DOT_ERR_PRINTF|DOT_LOG_PRINTF;
#else
    //m_flag = DOT_ERR_PRINTF|DOT_ERR_FILE|DOT_LOG_PRINTF|DOT_LOG_FILE;
    m_flag = DOT_ERR_PRINTF|DOT_LOG_PRINTF;
#endif
    //ctor
}

CDomSite::~CDomSite()
{
    //dtor
}

STDMETHODIMP_(void*) CDomSite::Query(IID iid)
{
    if(true == IID_CMP(iid,IID(ISite)))
        return dynamic_cast<ISite*>(this);
    return NULL;
}

STDMETHODIMP_(const char*) CDomSite::GetPath()
{
    return m_path.c_str();
}

STDMETHODIMP CDomSite::DumpSetFlag(DUMP_FLAG flag)
{
    m_flag = flag;
    return S_OK;
}

STDMETHODIMP_(ISite::DUMP_FLAG) CDomSite::DumpGetFlag()
{
    return m_flag;
}

STDMETHODIMP CDomSite::DumpSetLogLevelRange(int min,int max)
{
    JCHK2(DUMP_LOG_MIN_LEVEL <= min && min <= max,E_INVALIDARG,"min:%d max:%d invalid",min,max);
    m_min = min;
    m_max = max;
    return S_OK;
}

STDMETHODIMP CDomSite::DumpGetLogLevelRange(int* pMin,int* pMax)
{
    if(NULL != pMin)
        *pMin = m_min;
    if(NULL != pMax)
        *pMax = m_max;
    return S_OK;
}

STDMETHODIMP CDomSite::Trace(int level,const char* pFormat,...)
{
    JCHK(DUMP_LOG_MIN_LEVEL <= level,E_INVALIDARG);

	char decp[512] = {0};
	if(pFormat)
	{
		va_list va;
		va_start(va, pFormat);
		vsnprintf(decp,512,pFormat,va);
		va_end(va);
	}

    return InternalDump(m_path.c_str(),m_flag,m_min,m_max,level,NULL,0,NULL,decp);
}

STDMETHODIMP CDomSite::Check(HRESULT hr,const char* pFile,unsigned int line,const char* pCode,const char* pFormat,...)
{
    JCHK(IS_FAIL(hr),E_INVALIDARG);

	char decp[512] = {0};
	if(pFormat)
	{
		va_list va;
		va_start(va, pFormat);
		vsnprintf(decp,512,pFormat,va);
		va_end(va);
	}

    return InternalDump(m_path.c_str(),m_flag,m_min,m_max,hr,pFile,line,pCode,decp);
}
