#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include "stdafx.h"

class CHttpSession : public IHttpSession , public IEventCallback
{
public:
    DOM_DECLARE(CHttpSession)
    //IHttpSession

    //IEventCallback
    STDMETHODIMP OnEvent(uint32_t id,void* pParam,void* pTag);
    //CHttpSession
protected:
    dom_ptr<IStream> m_http;
    dom_ptr<IStream> m_file;
};

#endif // HTTPSESSION_H
