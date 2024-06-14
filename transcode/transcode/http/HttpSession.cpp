#include "HttpSession.h"

CHttpSession::CHttpSession()
{
    //ctor
}

bool CHttpSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(NULL != pParam,false);
    return true;
}

void CHttpSession::FinalDestructor(bool finally)
{
    printf("session:%p close\n",this);
}

DOM_QUERY_IMPLEMENT_BEG(CHttpSession)
DOM_QUERY_IMPLEMENT(IHttpSession)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CHttpSession::OnEvent(uint32_t id,void* pParam,void* pTag)
{
    return S_OK;
}
