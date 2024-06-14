#include "EventPoint.h"

CEventPoint::CEventPoint()
{
    //ctor
}

bool CEventPoint::FinalConstruct(Interface* pOuter,void* pParam)
{
    return true;
}

bool CEventPoint::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        m_it = NULL;
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CEventPoint)
DOM_QUERY_IMPLEMENT(IEventPoint)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(void) CEventPoint::Set(IEventCallback* callback)
{
    if(m_callback == callback)
        return;

    m_callback->Erase(NULL == callback);
    m_callback = callback;
}

STDMETHODIMP_(IEventCallback*) CEventPoint::Get()
{
    return m_callback;
}

STDMETHODIMP CEventPoint::Notify(uint32_t type,int32_t param1,void* param2,void* param3)
{
    return NULL == m_callback ? E_EOF : m_callback->OnEvent(m_this,NULL,type,param1,param2,param3);
}
