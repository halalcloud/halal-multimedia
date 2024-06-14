#include "EventPoint.h"

CEventPoint::CEventPoint()
{
    //ctor
}

bool CEventPoint::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(S_OK == NotifySet((Interface*)pParam),false);
    return true;
}

bool CEventPoint::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        m_callback = NULL;
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CEventPoint)
DOM_QUERY_IMPLEMENT(IEventPoint)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CEventPoint::NotifySet(Interface* obj)
{
    dom_ptr<IEventCallback> spCallback;
    if(NULL != obj)
    {
        JCHK(spCallback.QueryFrom(obj),E_INVALIDARG);
    }

    if(spCallback == m_callback)
        return S_OK;

    if(m_callback != NULL)
        m_callback->Set(NULL,spCallback != NULL);
    if(spCallback != NULL)
        spCallback->Set(GetOwner());
    m_callback = spCallback;
    return S_OK;
}

STDMETHODIMP_(IEventCallback*) CEventPoint::NotifyGet()
{
    return m_callback;
}

STDMETHODIMP CEventPoint::Notify(uint32_t type,int32_t param1,void* param2,void* param3)
{
    return NULL == m_callback ? E_EOF : m_callback->OnEvent(this,NULL,type,param1,param2,param3);
}
