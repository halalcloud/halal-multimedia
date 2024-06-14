#ifndef EVENTPOINT_H
#define EVENTPOINT_H
#include "stdafx.h"

class CEventPoint : public IEventPoint
{
public:
    DOM_DECLARE(CEventPoint)
    //IEventPoint
    STDMETHODIMP_(void) Set(IEventCallback* callback);
    STDMETHODIMP_(IEventCallback*) Get();
    STDMETHODIMP Notify(uint32_t type,int32_t param1,void* param2,void* param3);
protected:
    dom_ptr<IEventCallback> m_callback;
};

#endif // EVENTPOINT_H
