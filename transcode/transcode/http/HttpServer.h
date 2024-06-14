#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "stdafx.h"
struct create_info
{
    IServiceCallback* callback;
    IStreamServer* server;
};

class CHttpServer : public IServer , public IStreamCallback
{
public:
    CHttpServer();
    virtual ~CHttpServer();
    DOM_DECLARE
    //IEventCallback
    STDMETHODIMP OnEvent(uint32_t id,void* param1,int32_t param2,const int64_t& param3);
protected:
    IServiceCallback* m_pCallback;
    dom_ptr<IStreamServer> m_spStream;
};

#endif // HTTPSERVER_H
