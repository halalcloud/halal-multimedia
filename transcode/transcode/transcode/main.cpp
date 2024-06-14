#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <list>
#include <memory>
#include <dom.h>
#include <iStream.h>
#include <iProfile.h>
#include <iGraphBuilder.h>
#include "../src/Url.cpp"
#include <transcode.h>
int getch(void) {
    struct termios tm, tm_old;
    int fd = STDIN_FILENO, c;
    if(tcgetattr(fd, &tm) < 0)
    return -1;
    tm_old = tm;
    cfmakeraw(&tm);
    if(tcsetattr(fd, TCSANOW, &tm) < 0)
    return -1;
    c = fgetc(stdin);
    if(tcsetattr(fd, TCSANOW, &tm_old) < 0)
    return -1;
    return c;
}

ISite* g_pSite;
class CObjServer : public IEpollCallback
{
    struct CObjClient : public IEpollCallback , public IGraphBuilderCallback
    {
        friend class CObjServer;
        typedef list< CObjClient* > Set;
        typedef Set::iterator It;
        It it;
        CObjClient(CObjServer* pServer):m_pServer(pServer),m_status(ts_create)
        {

        }
        virtual ~CObjClient()
        {
            LOG(0,"transocde release close");
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
            if(m_spGB != NULL)
            {
                LOG(0,"transocde close stop");
                m_spGB->Stop();
                m_spGB = NULL;
            }
            m_session = NULL;
            m_status = ts_free;
        }
        HRESULT Open(IStreamServer* pServer)
        {
            //JCHK(m_session.Create(CLSID_CBufSession,dynamic_cast<IEpollCallback*>(this)),E_FAIL);
            return m_session->Open(pServer);
        }
        HRESULT Open(url* pUrl)
        {
            HRESULT hr;
            JCHK(m_session.Create(CLSID_CBufSession,dynamic_cast<IEpollCallback*>(this)),E_FAIL);
            JIF(m_session->Open(pUrl));

            dom_ptr<IProfile> spMsg;
            JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);

            uint16_t port = m_pServer->m_server->GetPort();
            JCHK(spMsg->Write(transcode_command_key,tcv_regist,sizeof(tcv_regist)),E_FAIL);
            JCHK(spMsg->Write(tcv_regist_app_key,"transocde",10),E_FAIL);
            JCHK(spMsg->Write(tcv_regist_port_key,port),E_FAIL);
            JIF(Deliver(spMsg));
            return hr;
        }
        template<class Q> HRESULT Deliver(dom_ptr<Q>& spObj)
        {
            HRESULT hr;
            if(false == m_session->IsOpen())
                return E_EOF;

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
            if(false == m_session->IsOpen())
                return E_EOF;

            dom_ptr<IBuffer> spBuf;
            dom_ptr<IStream> spStm;

            JIF(m_session->Receive(&spBuf));
            JCHK(spBuf.Query(&spStm),E_FAIL);

            JIF(spObj.Load(spStm));
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
                hr = Process(spMsg);
                if(IS_FAIL(hr))
                    OnEnd(hr,NULL);
            }
            else if(et_error == id)
            {
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
                LOG(0,"task[%s] recv[%s]",m_task_id.c_str(),cmd);
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_regist_dump_path_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);

                ISite::DUMP_FLAG flag = g_pSite->DumpGetFlag();
                flag |= DOT_ERR_FILE | DOT_LOG_FILE;
                g_pSite->DumpSetFlag(flag);
                g_pSite->DumpSetFile((const char*)pVal->value);
                //LOG(0,"transcode task create");
                m_status = ts_regist;
                LOG(0,"task[%s] finish[%s]",m_task_id.c_str(),cmd);
            }
            else if(STR_CMP(cmd,tcv_create))
            {
                LOG(0,"task[%s] recv[%s]",m_task_id.c_str(),cmd);
                IProfile::val* pVal;
                JCHK(pVal = pMsg->Read(tcv_task_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);
                m_task_id = (const char*)pVal->value;

                string path = g_pSite->DumpGetFile();
                path += m_task_id;
                path += ".json";

                JCHK(pVal = pMsg->Read(tcv_create_json_key),E_FAIL);
                JCHK(NULL != pVal->value,E_FAIL);

                FILE* pFile;
                JCHK1(pFile = fopen(path.c_str(),"w"),E_FAIL,"create file:[%s] fail",path.c_str());
                fwrite(pVal->value,1,pVal->len-1,pFile);
                fclose(pFile);

                JCHK(m_spGB.Create(CLSID_CGraphBuilder),E_FAIL);
                JIF(m_spGB->Play((const char*)pVal->value,false,dynamic_cast<IGraphBuilderCallback*>(this)));
                m_status = ts_running;
                m_task_return.clear();
                LOG(0,"task[%s] finish[%s]",m_task_id.c_str(),cmd);
            }
            else if(STR_CMP(cmd,tcv_cancel))
            {
                LOG(0,"task[%s] recv[%s]",m_task_id.c_str(),cmd);
                if(m_spGB == NULL)
                    return hr;
                LOG(0,"transcode cancel stop");
                m_spGB->Stop();
                m_spGB = NULL;
                m_status = ts_cancel;
                LOG(0,"task[%s] finish[%s]",m_task_id.c_str(),cmd);
            }
            else if(STR_CMP(cmd,tcv_quit))
            {
                LOG(0,"task[%s] recv[%s]",m_task_id.c_str(),cmd);
                m_status = ts_free;
                Close();
                LOG(0,"task[%s] finish[%s]",m_task_id.c_str(),tcv_quit);
                exit(m_status);
            }
            else if(STR_CMP(cmd,tcv_status))
            {
                //LOG(0,"task[%s] recv[%s]",m_task_id.c_str(),cmd);
                dom_ptr<IProfile> spMsg;
                JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);

                JCHK(spMsg->Write(transcode_command_key,tcv_status,sizeof(tcv_status)),E_FAIL);
                JCHK(spMsg->Write(tcv_task_key,m_task_id.c_str(),m_task_id.size()+1),E_FAIL);
                JCHK(spMsg->Write(tcv_status_key,m_status),E_FAIL);
                if(ts_cancel < m_status)
                {
                    if(false == m_task_return.empty())
                    {
                        JCHK(spMsg->Write(tcv_descr_key,m_task_return.c_str(),m_task_return.size()+1),E_FAIL);
                    }
                }
                else if(m_spGB != NULL)
                {
                    const char* msg = m_spGB->GetInfo(IGraphBuilder::it_status);
                    JCHK(spMsg->Write(tcv_descr_key,msg,strlen(msg)+1),E_FAIL);
                    printf("%s\n",msg);
                }
                JIF(Deliver(spMsg));
                //LOG(0,"task[%s] finish[%s]",m_task_id.c_str(),cmd);
            }
            else if(STR_CMP(cmd,tcv_input))
            {
                LOG(0,"task[id:%s] recv[%s]",m_task_id.c_str(),cmd);
                dom_ptr<IProfile> spMsg;
                JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
                JCHK(spMsg->Write(transcode_command_key,tcv_input,sizeof(tcv_input)),E_FAIL);
                JCHK(spMsg->Write(tcv_task_key,m_task_id.c_str(),m_task_id.size()+1),E_FAIL);
                const char* input = m_spGB->GetInfo(IGraphBuilder::it_input);
                JCHK(spMsg->Write(tcv_descr_key,input,strlen(input)+1),E_FAIL);
                JIF(Deliver(spMsg));
                LOG(0,"task[%s] finish[%s]",m_task_id.c_str(),cmd);
            }
            else
            {
                printf("receive unknown cmd:%s\n",cmd);
            }
            return hr;
        }
        STDMETHODIMP OnEnd(HRESULT hr,const char* json)
        {
            LOG(0,"task[id:%s] send end hr:%d json:%s",m_task_id.c_str(),hr,json);
            if(ts_free != m_status)
                m_status = S_OK <= hr ? ts_success : ts_fail;
            dom_ptr<IProfile> spMsg;
            JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
            JCHK(spMsg->Write(transcode_command_key,tcv_return,sizeof(tcv_return)),E_FAIL);
            JCHK(spMsg->Write(tcv_task_key,m_task_id.c_str(),m_task_id.size()+1),E_FAIL);
            JCHK(spMsg->Write(tcv_status_key,m_status),E_FAIL);
            if(NULL != json)
                m_task_return = json;
            if(false == m_task_return.empty())
            {
                JCHK(spMsg->Write(tcv_descr_key,m_task_return.c_str(),m_task_return.size()+1),E_FAIL);
            }
            return Deliver(spMsg);
        }
        STDMETHODIMP OnMsg(const char* json)
        {
            if(NULL == json)
                return S_OK;
            LOG(0,"task[id:%s] send msg json:%s",m_task_id.c_str(),json);
            dom_ptr<IProfile> spMsg;
            JCHK(spMsg.Create(CLSID_CMemProfile),E_FAIL);
            JCHK(spMsg->Write(transcode_command_key,tcv_message,sizeof(tcv_message)),E_FAIL);
            JCHK(spMsg->Write(tcv_task_key,m_task_id.c_str(),m_task_id.size()+1),E_FAIL);
            JCHK(spMsg->Write(tcv_descr_key,json,strlen(json)+1),E_FAIL);
            return Deliver(spMsg);
        }
    protected:
        ClassInfo m_info;
        CObjServer* m_pServer;
        dom_ptr<IBufSession> m_session;
        dom_ptr<IGraphBuilder> m_spGB;
        string m_task_id;
        string m_task_return;
        transcode_status m_status;
    };
public:
    CObjServer(){}
    ~CObjServer()
    {
        CObjClient::It it;
        while(m_clients.end() != (it = m_clients.begin()))
        {
            CObjClient* pClient;
            if(NULL != (pClient = *it))
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
        HRESULT hr = S_OK;

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
        pClient->it = m_clients.insert(m_clients.end(),pClient);
        JIF(pClient->Open(&url));
        return hr;
    }
    STDMETHODIMP OnEvent(uint32_t id,void* pParam)
    {
        HRESULT hr = S_OK;
        if(IStreamServer::accept == id)
        {
            JCHK(false == m_clients.empty(),E_AGAIN);
            hr = m_clients.front()->Open(m_server);
        }
        else if(et_timeout == id)
        {
            if(true == m_clients.empty())
            {
                LOG(0,"transcoder app timeout quit");
                exit(ts_create);
            }
            CObjClient* pClient = m_clients.front();
            if(ts_running > pClient->m_status)
            {
                LOG(0,"transcoder app run task timeout quit");
                //exit(pClient->m_status);
            }
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
protected:
    ClassInfo m_info;
    dom_ptr<IEpoll> m_epoll;
    dom_ptr<IStreamServer> m_server;
    CObjClient::Set m_clients;
};

dom_ptr<IGraphBuilder> g_spGB;
int main(int argc,char *argv[])
{
    if(argc < 2)
    {
        printf("---------------transcode-------------\n");
        printf("Useage:%s [node url/json file]\n",strrchr(argv[0],'/')+1);
        return 0;
    }
    HRESULT hr = S_OK;
    if(NULL == (g_pSite = GetSite()))
        return hr;

    CUrl url(argv[1]);

    if(NULL != url.protocol)
    {
        ISite::DUMP_FLAG flag = g_pSite->DumpGetFlag();
        flag |= DOT_ERR_FILE | DOT_LOG_PRINTF;
        g_pSite->DumpSetFlag(flag);
        g_pSite->Load("./");  //load ./ all plug

        CObjServer server;
        JIF(server.Startup("obj://localhost:4321"));
        JIF(server.Append(argv[1]));
        printf("---------------press esc to exit-------------\n");
        while('\n' != getchar()){}
    }
    else
    {
        ISite::DUMP_FLAG flag = g_pSite->DumpGetFlag();
        flag |= DOT_ERR_FILE | DOT_LOG_FILE;
        g_pSite->DumpSetFlag(flag);
        g_pSite->Load("./");  //load ./ all plug
        JCHK(g_spGB.Create(CLSID_CGraphBuilder),E_FAIL);
        if(argv[1][0] != '{')
        {
            ifstream ifs(url.path);
            string json((istreambuf_iterator<char>(ifs)),
                std::istreambuf_iterator<char>());
            g_spGB->Play(json.c_str());
            while(g_spGB->IsRunning())
            {
                sleep(1);
                const char* msg = g_spGB->GetInfo(IGraphBuilder::it_status);
                if(NULL != msg)
                    printf("status:%s\n",msg);
            }
            while('\n' != getchar()){}
        }
        else
        {
            g_spGB->Play(argv[1],true);
        }
    }
    return hr;
}
