#include <fstream>
#include <sstream>
#include <map>
#include <time.h>
#include <memory>
#include <dom.h>
#include <iStream.h>
#include <iProfile.h>
#include <iGraphBuilder.h>
#include "../src/Url.cpp"
#include <transcode.h>

ISite* g_pSite;
string g_task_path;
class CObjServer : public IEpollCallback
{
    struct CObjClient : public IEpollCallback
    {
        friend class CObjServer;
        typedef map< string,CObjClient* > Set;
        typedef Set::iterator It;
        typedef pair<Set::key_type,Set::mapped_type> Pair;
        It it;
        CObjClient(CObjServer* pServer):m_pServer(pServer),m_end(false),m_status(ts_create)
        {
        }
        virtual ~CObjClient()
        {
            Close();
        }
        //Interface
        STDMETHODIMP_(const ClassInfo&) Class()
        {
            return m_info;
        }
        STDMETHODIMP_(void*)Query(IID iid)
        {
            return this;
        }
        STDMETHODIMP_(REFTYPE) AddRef()
        {
            return 1;
        }
        STDMETHODIMP_(REFTYPE) Release()
        {
            return 1;
        }
        void Close()
        {
            m_session = NULL;
        }
        HRESULT Open(IStreamServer* pServer)
        {
            HRESULT hr;
            JCHK(m_session.Create(CLSID_CBufSession,this),E_FAIL);
            JIF(m_session->Open(pServer));
            char strTime[20];
            time_t long_time = time(NULL);
            const tm* pTM = localtime(&long_time);
            snprintf(strTime,20,"%04d-%02d-%02d_%02d-%02d-%02d",
                pTM->tm_year+1900,pTM->tm_mon+1,pTM->tm_mday,pTM->tm_hour,pTM->tm_min,pTM->tm_sec);
            m_task_id = strTime;
            return hr;
        }
        HRESULT Open(url* pUrl)
        {
            HRESULT hr;
            JCHK(m_session.Create(CLSID_CBufSession,this),E_FAIL);
            JIF(m_session->Open(pUrl));
            return hr;
        }
        template<class Q> HRESULT Deliver(dom_ptr<Q>& spObj)
        {
            HRESULT hr;
            dom_ptr<IBuffer> spBuf;
            dom_ptr<IStream> spStm;

            JCHK(spBuf.Create(CLSID_CBuffer),E_FAIL);
            JCHK(spBuf.Query(&spStm),E_FAIL);

            JIF(spObj.Save(spStm));
            JIF(spBuf->Alloc(spBuf->size));
            JIF(spObj.Save(spStm));
            JIF(m_session->Deliver(spBuf));

            return hr;
        }
        template<class Q> HRESULT Receive(dom_ptr<Q>& spObj)
        {
            HRESULT hr;
            dom_ptr<IBuffer> spBuf;
            dom_ptr<IStream> spStm;

            JIF(m_session->Receive(&spBuf));
            JCHK(spBuf.Query(&spStm),E_FAIL);

            JIF(spObj.Load(spStm));
            return hr;
        }
        HRESULT Quit()
        {
            HRESULT hr = S_OK;
            if(false == m_end)
            {
                dom_ptr<IProfile> spMsg;
                JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
                JCHK(spMsg->Write(transcode_command_key,tcv_quit,sizeof(tcv_quit)),E_FAIL);
                printf("send %s cmd\n",tcv_quit);
                JIF(Deliver(spMsg));
                m_end = true;
            }
            return hr;
        }
        STDMETHODIMP OnEvent(uint32_t id,void* pParam)
        {
            HRESULT hr = S_OK;
            if(IBufSession::receive == id)
            {
                dom_ptr<IProfile> spMsg;
                JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
                JIF(Receive(spMsg));
                Process(spMsg);
            }
            else if(et_error == id)
            {
                m_status = ts_fail;
            }
            return hr;
        }
        STDMETHODIMP_(IEpoll*) GetEpoll()
        {
            return m_pServer->GetEpoll();
        }
        HRESULT Process(IProfile* pMsg)
        {
            HRESULT hr = S_OK;
            IProfile::val* pVal;
            JCHK(pVal = pMsg->Read(transcode_command_key),E_FAIL);
            const char* cmd = (const char*)pVal->value;

            if(STR_CMP(cmd,tcv_regist))
            {
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_regist_app_key),E_FAIL);
                JCHK(pMsg->Read(tcv_regist_port_key,m_app_port),E_FAIL);
                m_app_type = (const char*)pVal->value;
                printf("recv %s cmd,app:%s service port:%d\n",cmd,(const char*)pVal->value,m_app_port);

                m_app_dump_path = "./app/";
                m_app_dump_path += m_app_type;
                m_app_dump_path += "/";

                dom_ptr<IProfile> spMsg;
                JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
                JCHK(spMsg->Write(transcode_command_key,tcv_regist,sizeof(tcv_regist)),E_FAIL);
                JCHK(spMsg->Write(tcv_regist_dump_path_key,m_app_dump_path.c_str(),m_app_dump_path.size()+1),E_FAIL);
                printf("send %s cmd,dump path:%s\n",cmd,m_app_dump_path.c_str());
                JIF(Deliver(spMsg));

                ifstream ifs(g_task_path.c_str());
                string json((istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());

                JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
                JCHK(spMsg->Write(transcode_command_key,tcv_create,sizeof(tcv_create)),E_FAIL);
                JCHK(spMsg->Write(tcv_task_key,m_task_id.c_str(),m_task_id.size()+1),E_FAIL);
                JCHK(spMsg->Write(tcv_create_json_key,json.c_str(),json.size()+1),E_FAIL);
                printf("send %s cmd,task id:%s json:%s\n",cmd,m_task_id.c_str(),json.c_str());
                JIF(Deliver(spMsg));
                m_time = 0;
            }
            else if(STR_CMP(cmd,tcv_status))
            {
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_task_key),E_FAIL);
                if(false == STR_CMP((char*)pVal->value,m_task_id.c_str()))
                {
                    m_task_id = (char*)pVal->value;
                }
                JCHK(pMsg->Read(tcv_status_key,m_status),E_FAIL);
                printf("recv %s cmd,task id:%s status:%d\t",cmd,m_task_id.c_str(),(int)m_status);
                if(NULL != (pVal = pMsg->Read(tcv_descr_key)))
                    printf("descr:%s\n",(char*)pVal->value);
                if(ts_cancel < m_status)
                {
                    JIF(Quit());
                }
            }
            else if(STR_CMP(cmd,tcv_return))
            {
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_task_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);
                if(false == STR_CMP((char*)pVal->value,m_task_id.c_str()))
                {
                    m_task_id = (char*)pVal->value;
                }
                JCHK(pMsg->Read(tcv_status_key,m_status),E_FAIL);
                if(NULL != (pVal = pMsg->Read(tcv_descr_key)))
                {
                    printf("recv %s cmd,status:%d,task id:%s descr:%s\n",cmd,m_status,m_task_id.c_str(),(const char*)pVal->value);
                }
                else
                {
                    printf("recv %s cmd,status:%d,task id:%s \n",cmd,m_status,m_task_id.c_str());
                }
                JIF(Quit());
            }
            else if(STR_CMP(cmd,tcv_warring))
            {
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_task_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);
                const char* pTaskID = (const char*)pVal->value;
                JCHK(pVal = pMsg->Read(tcv_descr_key),E_FAIL);
                printf("recv %s cmd,task id:%s descr:%s\n",cmd,pTaskID,(const char*)pVal->value);
            }
            else if(STR_CMP(cmd,tcv_error))
            {
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_task_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);
                const char* pTaskID = (const char*)pVal->value;
                JCHK(pVal = pMsg->Read(tcv_descr_key),E_FAIL);
                printf("recv %s cmd,task id:%s descr:%s\n",cmd,pTaskID,(const char*)pVal->value);
                hr = S_OK;
            }
            else if(STR_CMP(cmd,tcv_input))
            {
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_task_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);
                const char* pTaskID = (const char*)pVal->value;
                JCHK(pVal = pMsg->Read(tcv_descr_key),E_FAIL);
                printf("recv %s cmd,task id:%s descr:%s\n",cmd,pTaskID,(const char*)pVal->value);
            }
            else if(STR_CMP(cmd,tcv_message))
            {
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_task_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);
                const char* pTaskID = (const char*)pVal->value;
                JCHK(pVal = pMsg->Read(tcv_descr_key),E_FAIL);
                printf("recv %s cmd,task id:%s descr:%s\n",cmd,pTaskID,(const char*)pVal->value);
            }
            else
            {
                printf("receive unknown cmd:%s\n",cmd);
            }
            return hr;
        }
        HRESULT GetStatus()
        {
            HRESULT hr = S_FALSE;
            if(m_status <= ts_running)
            {
                dom_ptr<IProfile> spMsg;
                JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
                JCHK(spMsg->Write(transcode_command_key,tcv_status,sizeof(tcv_status)),E_FAIL);
                printf("send %s cmd status\n",tcv_status);
                JIF(Deliver(spMsg));
            }
            else
            {
                printf("client task id:%s current status:%d will be delete\n",m_task_id.c_str(),m_status);
            }
            return hr;
        }
    protected:
        ClassInfo m_info;
        CObjServer* m_pServer;
        dom_ptr<IBufSession> m_session;
        string m_app_type;
        uint16_t m_app_port;
        string m_app_dump_path;
        bool m_end;
        transcode_status m_status;
        double m_progress;
        string m_status_descr;
        string m_task_id;
        uint32_t m_time;
    };
public:
    CObjServer(){}
    ~CObjServer()
    {
        CObjClient::It it;
        while(m_clients.end() != (it = m_clients.begin()))
        {
            CObjClient* pClient;
            if(NULL != (pClient = it->second))
                delete pClient;
            m_clients.erase(it);
        }
        m_server->Shutdown();
    }
    //Interface
    STDMETHODIMP_(const ClassInfo&) Class()
    {
        return m_info;
    }
    STDMETHODIMP_(void*)Query(IID iid)
    {
        return this;
    }
    STDMETHODIMP_(REFTYPE) AddRef()
    {
        return 1;
    }
    STDMETHODIMP_(REFTYPE) Release()
    {
        return 1;
    }
    HRESULT Startup(const char* pUrl)
    {
        HRESULT hr;
        JCHK(NULL != pUrl,E_INVALIDARG);
        JCHK(m_server == NULL,E_FAIL);

        CUrl url(pUrl);
        JCHK(m_epoll.Create(CLSID_CEpoll),E_FAIL);
        JIF(m_epoll->SetTimeout(this,5000));
        JCHK(m_server.Create(CLSID_CNetworkServer,this,&url),E_FAIL);
        return hr;
    }
    HRESULT Append(const char* pUrl)
    {
        HRESULT hr;
        JCHK(NULL != pUrl,E_INVALIDARG);
        JCHK(m_epoll != NULL,E_FAIL);

        CUrl url(pUrl);
        CObjClient* pClient;
        JCHK(pClient = new CObjClient(this),E_OUTOFMEMORY);
        hr = pClient->Open(&url);
        if(IS_FAIL(hr))
        {
            delete pClient;
            return hr;
        }
        pair<CObjClient::It,bool> result = m_clients.insert(CObjClient::Pair(pClient->m_task_id,pClient));
        if(false == result.second)
            delete pClient;
        else
            pClient->it = result.first;
        return hr;
    }
    STDMETHODIMP OnEvent(uint32_t id,void* pParam)
    {
        HRESULT hr = S_OK;
        if(et_timeout == id)
        {
            printf("5000ms time out\n");
            CObjClient::It it = m_clients.begin();
            while(it != m_clients.end())
            {
                CObjClient* pClient;
                if(NULL != (pClient = it->second))
                {
                    if(S_OK != pClient->GetStatus())
                    {
                        printf("delete client task id:%s\n",pClient->m_task_id.c_str());
                        delete pClient;
                        CObjClient::It temp = it++;
                        m_clients.erase(temp);
                        continue;
                    }
                }
                ++it;
            }
        }
        else if(IStreamServer::accept == id)
        {
            //printf("server port:%d socket:%d accept connection socket:%d\n",m_port,m_socket,socket);
            CObjClient* pClient;
            JCHK(pClient = new CObjClient(this),E_OUTOFMEMORY);
            hr = pClient->Open(m_server);
            if(IS_FAIL(hr))
            {
                delete pClient;
                return hr;
            }

            pair<CObjClient::It,bool> result = m_clients.insert(CObjClient::Pair(pClient->m_task_id,pClient));
            if(false == result.second)
                delete pClient;
            else
                pClient->it = result.first;
        }
        else if(et_error == id)
        {
        }
        return hr;
    }
    STDMETHODIMP_(IEpoll*) GetEpoll()
    {
        return m_epoll.p;
    }
    HRESULT Cancel(const string& task_id)
    {
        CObjClient::It it = m_clients.find(task_id);
        CObjClient* pClient = it->second;
        return pClient->Quit();
    }
protected:
    ClassInfo m_info;
    dom_ptr<IEpoll> m_epoll;
    dom_ptr<IStreamServer> m_server;
    CObjClient::Set m_clients;
};

int main(int argc,char *argv[])
{
    if(argc < 2)
        return E_FAIL;

    HRESULT hr = S_OK;

    if(NULL == (g_pSite = GetSite()))
        return hr;

    ISite::DUMP_FLAG flag = g_pSite->DumpGetFlag();
    flag |= DOT_ERR_FILE | DOT_LOG_PRINTF | DOT_LOG_FILE;
    g_pSite->DumpSetFlag(flag);
    g_pSite->Load("./");  //load ./ all plug

    g_task_path = argv[1];

    CObjServer server;
    JIF(server.Startup("obj://localhost:1234"));
    if(argc > 2)
    {
        server.Append(argv[2]);
    }

    int c;
    while ((c = getchar()) != '\n'){}


    return hr;
}
