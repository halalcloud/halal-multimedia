#include "HttpServer.h"
#include "HttpClient.h"

CHttpServer::CHttpServer()
:m_pCallback(NULL)
{
    //ctor
}

CHttpServer::~CHttpServer()
{
    //dtor
    m_spStream = NULL;
}

bool CHttpServer::FinalConstruct(void* pParam)
{
    JCHK(NULL != pParam,false);
    create_param* pP = (create_param*)pParam;
    m_pCallback = pP->callback;
    network_create_param param;
    param.callback = dynamic_cast<IStreamCallback*>(this);
    param.pUrl = pP->pUrl;
    param.type = network_create_param::tcp;
    return m_spStream.Create(CLSID_CNetworkServer,NULL,&param);
}

DOM_QUERY_IMPLEMENT_BEG(CHttpServer)
DOM_QUERY_IMPLEMENT(IServer)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CHttpServer::Aggregate(Interface* pOuter)
{
    return S_OK;
}

STDMETHODIMP CHttpServer::OnEvent(uint32_t id,void* param1,int32_t param2,const int64_t& param3)
{
    if(id == IStreamServer::create)
        return m_pCallback->OnCreate(param2,(IStreamCallback*)param1);
    if(id == IStreamServer::acceptting)
    {
        create_info info;
        info.callback = m_pCallback;
        info.server = m_spStream.p;
        dom_ptr<IClient> spClient;
        JCHK0(spClient.p = static_cast<IClient*>(CHttpClient::Create(IID(IClient),this,&info)),E_FAIL,"http server accept tcp client fail");
        return m_pCallback->OnAccept(spClient);
    }
    return S_OK;
}
