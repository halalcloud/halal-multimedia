#include "HttpSession.h"
#include <Url.cpp>
#include <fstream>
#include <sstream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>

#include "DHttpHeader.hpp"
uint64_t GetMSCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

CHttpSession::CHttpSession()
    :m_type(FT_None)
    ,m_flag(0)
    ,m_status(S_Stop)
    ,m_pTag(NULL)
    ,m_mode(0)
    ,m_next(false)
    ,m_keep_alive(false)
    ,_status(RECV_HTTP_H)
    ,_offsetD(-1)
    ,_offsetP(-1)
    ,_isRes(false)
    ,_contentLen(-1)
{
    //ctor
}

bool CHttpSession::FinalConstruct(Interface* pOuter,void* pParam)
{
    if(NULL != pParam)
    {
        JCHK(m_spStream.Create(CLSID_CNetworkStream,(ICallback*)this,false,pParam),false);
    }
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true),false);
    JCHK(m_spAllocate.Create(CLSID_CMediaFrameAllocate),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);

    //printf("accept\t");
    return true;
}

bool CHttpSession::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        if(S_Play == m_status)
        {
            Notify(S_Pause);
        }
        if(S_Stop != m_status);
        {
            Notify(S_Stop);
        }
        if(m_spStream != NULL)
            m_spStream->Close();
        LOG(0,"%s[%p] name:%s release",Class().name,(IFilter*)this,GetName());
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CHttpSession)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CHttpSession::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    HRESULT hr = S_OK;
    if(NULL == pUrl)
        pUrl = m_name.c_str();


    JIF(m_url.Set(pUrl));
    if(m_spStream == NULL)
    {
        JCHK2(m_url.m_protocol == HTTP_PROTOCOL_NAME,E_INVALIDARG,"url:[%s] not support protocol:[%s]",pUrl,m_url.m_protocol.c_str());
        JCHK(m_spStream.Create(CLSID_CNetworkStream,(ICallback*)this),E_FAIL);
    }
    else if(true == m_spStream->IsOpen())
    {
        m_next = true;
        m_name = pUrl;
        LOG(0,"CHttpSession::Load next:%s",pUrl);
        return hr;
    }

    LOG(0,"CHttpSession::Load:%s",pUrl);

    m_name = m_url.GetStreamID();
    //
    m_keep_alive = false;
    _status = RECV_HTTP_H;
    _offsetD =-1;
    _offsetP=-1;
    _isRes = false;
    if (_httpParser.get() != NULL)
        _httpParser.release();
    _buf.clear();

    JIF(m_spStream->Open(m_url.Get(),(uint32_t)m_type));
    //
    if(FT_Source == mode)
    {
        if (m_demuxer == NULL)
        {
            JIF(SetType((FilterType)mode));
            JIF(m_ep->Notify(ET_Filter_Build,0,(IFilter*)this));

        }

        //send get request to remote
        string header("GET ");

        // format path
        header += m_url.m_path;
        header += m_url.m_file;
        if(false == m_url.m_format.empty())
        {
            header += '.';
            header += m_url.m_format;
        }
        header += " ";
        header += "HTTP/1.1\r\n";

        header += "Host: ";
        header += m_url.m_host;
        header += "\r\n";

        if (m_url.m_port != 0 && m_url.m_port != HTTP_PORT)
        {
            header += "port: ";
            string sPort;
            try
            {
                sPort = boost::lexical_cast<string>(m_url.m_port);
            }
            catch(boost::bad_lexical_cast& e) {}
            header += sPort;
            header += "\r\n";
        }

        header += "\r\n";

        // only once

        _isRes = true;
        std::cout << "load: " << header <<endl;
        JIF(m_spStream->Write(header.c_str(), header.length()));
    }
    m_mode = mode;
    return hr;
}

STDMETHODIMP_(FilterType) CHttpSession::GetType()
{
    return m_type;
}

STDMETHODIMP CHttpSession::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CHttpSession::GetName()
{
    return true == m_name.empty() ? NULL : m_name.c_str();
}

STDMETHODIMP_(uint32_t) CHttpSession::GetFlag()
{
    return m_flag | m_spStream->GetFlag();
}

STDMETHODIMP_(uint32_t) CHttpSession::GetInputPinCount()
{
    return m_pinIn == NULL ? 0 : 1;
}

STDMETHODIMP_(IInputPin*) CHttpSession::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn.p : NULL;
}

STDMETHODIMP_(uint32_t) CHttpSession::GetOutputPinCount()
{
    return m_pinOut == NULL ? 0 : 1;
}

STDMETHODIMP_(IOutputPin*) CHttpSession::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut.p : NULL;
}

STDMETHODIMP_(IInputPin*) CHttpSession::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CHttpSession::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CHttpSession::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(FT_Render == m_type)
            {
                if(S_Stop == m_status)
                {
                }
                else if(S_Stop == cmd)
                {
                    if(m_spStream != NULL)
                        m_spStream->Close();
                }
            }
            else if(FT_Source == m_type)
            {
                if(S_Stop == m_status)
                {
                }
                else if(S_Stop == cmd)
                {
                    if(m_spStream != NULL)
                        m_spStream->Close();
                }
            }
            m_status = (Status)cmd;
            if(FT_Render == m_type)
            {
                if(S_Play == cmd)
                {
                    JCHK(m_pinIn != NULL,E_FAIL);

                    CUrl url;
                    JIF(url.SetStreamID(NULL,m_name.c_str()));
                    string name = url.GetStreamName();
                    string file = m_url.GetStreamName();
                    m_pinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);

                    //printf("buffer:%s\n",file.c_str());

                    if(name == file)
                    {
//                        LOG(4,"this:%p m3u8 send",this);
                        dom_ptr<IMediaType> spMt;
                        JCHK(spMt = m_pinIn->GetMediaType(),E_FAIL);

                        dom_ptr<IMediaFrame> spFrame;
                        if(NULL != (spFrame = spMt->GetFrame()))
                        {
                            JIF(m_pinIn->Write(spFrame));
                            if(0 != (spFrame->info.flag&MEDIA_FRAME_FLAG_EOF))
                                return S_OK;
                        }
                        JIF(m_ep->Notify(ET_Filter_Buffer,0,m_pinIn.p,NULL));
                    }
                    else
                    {
                        m_file = file;
                        //printf("%s begin\t",m_file.c_str());
                        hr = m_ep->Notify(ET_Filter_Buffer,0,m_pinIn.p,(void*)file.c_str());
                        if(S_OK > hr)
                        {
                            string header = get_response_header(404,m_url.m_format.c_str());
                            JIF(m_spStream->Write(header.c_str(),header.size(),IStream::WRITE_FLAG_EOF));
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(IFilter::C_Profile == cmd)
        {
        }
        else if(IFilter::C_Accept == cmd)
        {
            JCHK(m_spStream != NULL,E_FAIL);
            JIF(m_spStream->SetEventEnable(true));
            //printf("%p accept\n",this);
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CHttpSession::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CHttpSession::SetTag(void* tag)
{
    m_pTag = tag;
}

STDMETHODIMP_(void*) CHttpSession::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CHttpSession::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CHttpSession::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    if(NULL != pMT)
    {
        return MMT_DATA == pPin->GetMediaType()->GetMajor() ? S_OK : E_INVALIDARG;
    }
    else
    {
        return S_OK;
    }
}

STDMETHODIMP CHttpSession::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    if(NULL != pMT)
    {
        JCHK(m_demuxer.QueryFrom(pPinIn->GetFilter()),E_FAIL);
    }
    else if(NULL != m_demuxer)
        m_demuxer = NULL;
    return S_OK;
}

FILE* g_fpHttp = fopen("http.ts", "wb");
STDMETHODIMP CHttpSession::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    //printf("CHttpSession::OnWriteFrame:%ld\n",pFrame->info.dts);
    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != pFrame,E_INVALIDARG);

    HRESULT hr = S_OK;
    if (GetType() == FT_Source)
    {
    }
    else if (GetType() == FT_Render)
    {
        if(false == m_spStream->CanWrite())
            return E_AGAIN;
        dom_ptr<IMediaFrame> spFrame;
        if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
        {
            string header;
            dom_ptr<IStream> spStream;
            JCHK(spFrame.Create(CLSID_CMediaFrame),E_FAIL);
            JCHK(spFrame.Query(&spStream),E_FAIL);
            spFrame->info = pFrame->info;
            if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_EOF))
            {
                uint32_t contLen;
                JCHK(pFrame->GetBuf(0, NULL,&contLen),E_FAIL);
                header = get_response_header(200,m_url.m_format.c_str(),contLen);

            }
            else
            {
                IMediaType* pMT = pPin->GetMediaType();
                int64_t len = 0;
                pMT->GetStreamInfo(NULL,&len);
                if(len > 0)
                {
                    header = get_response_header(200,m_url.m_format.c_str(),(uint32_t)len);
                }
                else
                {
                    header = get_response_header(200,m_url.m_format.c_str());
                }
            }
            JIF(spStream->Write(header.c_str(),header.size()));
            JIF(spStream->Write(pFrame,0,IStream::WRITE_FLAG_FRAME|IStream::WRITE_FLAG_REFFER));
        }
        else
            spFrame = pFrame;
        hr = m_spStream->Write(spFrame,0,IStream::WRITE_FLAG_FRAME);
        m_ep->Notify(ET_Filter_Render,hr,m_pinIn.p,pFrame);
    }
    return hr;
}

//IEventCallback
STDMETHODIMP CHttpSession::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(ET_EOF == type)
    {
        if(false == m_file.empty())
        {
            //printf("%s end\n",m_file.c_str());
            m_file.clear();
        }
        if(E_HANGUP == param1)
        {
            if(true == m_next)
            {
                m_next = false;
                return Load(NULL,m_mode);
            }
            else
                return S_OK;
        }
    }
    else if(ET_Stream_Read == type)
    {
        return Recv(source, it, type, param1, param2, param3);
    }
    else if(ET_Stream_Write == type)
    {
        if(NULL == m_pinIn)
            return E_AGAIN;

        HRESULT hr;
        JIF(m_pinIn->Write(NULL));

        param2 = m_pinIn.p;
    }
    return m_ep->Notify(type,param1,param2,param3);
}

HRESULT CHttpSession::SetType(FilterType type)
{
    JCHK(FT_Source == type || FT_Render == type,S_OK);
    JCHK(FT_None == m_type,E_FAIL);

    HRESULT hr = S_OK;
    if(type == m_type)
        return hr;

    dom_ptr<IMediaType> spMT;
    JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
    JIF(spMT->SetMajor(MMT_DATA));

    if(FT_Source == type)
    {
        JCHK(m_pinOut.Create(CLSID_COutputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinOut->SetMediaType(spMT));
        if(0 < _contentLen)
        {
            JIF(spMT->SetStreamInfo(NULL,&_contentLen))
        }
    }
    else if(FT_Render == type)
    {
        spMT->SetSub(m_url.m_format.c_str());
        JCHK(m_pinIn.Create(CLSID_CInputPin,(IFilter*)this),E_FAIL);
        JIF(m_pinIn->SetMediaType(spMT));
    }
    m_type = type;
    return hr;
}

string urlDecode(string &SRC)
{
    string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++)
    {
        if (int(SRC[i])==37)
        {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        }
        else
        {
            ret+=SRC[i];
        }
    }
    return (ret);
}
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string.hpp>

HRESULT CHttpSession::Recv(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    HRESULT hr = S_OK;
    static const char * const POST = "POST";
    static const char* const GET = "GET";
    static const char*  CONNECTION = "Connection";
    static const char* const  CONTENT_LENGTH = "Content-Length";
    static const char* const  TRANSFER_ENCODING = "Transfer-Encoding";
    static const char* const CHUNKED = "chunked";
    static const char* const  CLOSE = "Close";
    static const char* const KEEP_ALIVE = "Keep-Alive";
    static const char* const  HOST = "Host";
    static const char* const  CONTENT_TYPE = "Content-Type";

    switch(_status)
    {
    case RECV_HTTP_H:
    {
        while(1)
        {
            char buf[1];
            JIF(m_spStream->Read(buf, 1));
            _buf.push_back(buf[0]);
            if (_buf.find("\r\n\r\n") != std::string::npos)
            {
                break;
            }
            if (_buf.size() >= 2048)
            {
                LOG(0, "already read 2048 bytes,but not found http header!");
                return E_FAIL;
            }
        }
        // 转换为小写
        //transform(_buf.begin(), _buf.end(),_buf.begin(), ::tolower);

        if (_httpParser.get() == nullptr)
        {
            if (_isRes)
                _httpParser.reset(new HttpParser(HTTP_RESPONSE));
            else
                _httpParser.reset(new HttpParser(HTTP_REQUEST));

            _alreadyRead = m_spStream->GetStatus().read_total_size;
        }
        if (-1 == _httpParser->parse(_buf.c_str(), _buf.size()))
        {
            LOG(0, "parser http header error, %s", _buf.c_str());
            return E_FAIL;
        }
        _buf.clear();
        if (false == _httpParser->isRequest()) // recv response
        {
            _contentLen = 0;
            string len;
            if (false == (len = _httpParser->feild(CONTENT_LENGTH)).empty())
            {
                _status = RECV_CONTENT;
                _contentLen = boost::lexical_cast<uint32_t>(len);
                return S_OK;
            }
            if (_httpParser->feild(TRANSFER_ENCODING).compare(CHUNKED) == 0)
                _status = RECV_CHUK_H;
            LOG(0, "recv response not include content-length");
            return S_OK;
        }

        if (_httpParser->isRequest()
                && 0 == _httpParser->method().compare(POST)) // recv post
        {
            string ct = _httpParser->feild(CONTENT_TYPE);
            string url = "http://";
            url += _httpParser->feild(HOST);
            url += _httpParser->getUrl();

            JIF(m_url.Set(url.c_str()));
            m_name = m_url.GetStreamID();
            JIF(SetType(FT_Source));

            dom_ptr<IProfile> spProfile;
            JCHK(spProfile.QueryFrom(m_pinOut->GetMediaType()),E_FAIL);
            IProfile::val* pVal = spProfile->Write("params", IID(IProfile), NULL, 0);
            JCHK(NULL != pVal && NULL != pVal->value,E_FAIL);
            IProfile* profile = (IProfile*)pVal->value;
            string v = _httpParser->getUrl();
            JCHK(profile->Write("path", v.c_str(), v.size()),E_FAIL);
            v = _httpParser->method();
            JCHK(profile->Write("method", v.c_str(), v.size()),E_FAIL);

            if (_httpParser->feild(TRANSFER_ENCODING).compare(CHUNKED) == 0)
            {
                _status = RECV_CHUK_H;
            }

            if ((v = _httpParser->feild(CONTENT_LENGTH)).empty() == false)
            {
                _status = RECV_CONTENT;
                _contentLen = boost::lexical_cast<uint32_t>(v);
                m_flag = 0;
            }
            else
            {
                _contentLen = 0;
                m_flag = FLAG_LIVE;
            }
            v = _httpParser->feild(CONNECTION);
            JCHK(profile->Write("connection", v.c_str(), v.size()),E_FAIL);


            if (_status == RECV_HTTP_H)
                _status = RECV_DIRECT2;

            JIF(m_ep->Notify(ET_Session_Push,0,m_pinOut.p));
            JIF(m_ep->Notify(ET_Filter_Build));

//            IInputPin* pPinIn;
//            JCHK(pPinIn = m_pinOut->GetConnection(),E_FAIL);
//            JCHK(m_demuxer.QueryFrom(pPinIn->GetFilter()),E_FAIL);
            return hr;
        }
        else if (_httpParser->isRequest()
                 && 0 == _httpParser->method().compare(GET)) // recv get
        {
            m_keep_alive = false;
            if (_httpParser->feild(CONNECTION).compare(KEEP_ALIVE))
                m_keep_alive = true;

            string ct = _httpParser->feild(CONTENT_TYPE);
            string url = "http://";
            url += _httpParser->feild(HOST);
            url += _httpParser->getUrl();
            JIF(m_url.Set(url.c_str()));
            //LOG(0,"%p:Recv::get %s content type:%s keep-alive:%d",this,url.c_str(),ct.c_str(),m_keep_alive);
            if(m_pinIn == NULL)
            {
                JIF(SetType(FT_Render));
                m_name = m_url.GetStreamID();
                if(0 <= m_url.m_pos)
                    m_flag &= ~FLAG_LIVE;
                else
                    m_flag |= FLAG_LIVE;
//                if(m_url.m_format == "ts")
//                    printf("pull:%s\n",url.c_str());
//                LOG(4,"this:%p ET_Session_Pull:%s",this,url.c_str());
//                uint64_t time = GetMSCount();
                hr = m_ep->Notify(ET_Session_Pull,0,m_pinIn.p);
//                printf("ET_Session_Pull use %lu ms",GetMSCount() - time);
            }
            return hr;
        }
        else // others
            return E_FAIL;



        break;
    }
    case RECV_CHUK_H:
    {
        while(1)
        {
            char buf[1];
            JIF(m_spStream->Read(buf, 1));
            _buf.push_back(buf[0]);
            if (_buf.find("\r\n") != std::string::npos)
            {
                _status = RECV_CHUK_B;
                _buf.clear();
                return S_OK;
            }
        }
        break;
    }
    case RECV_CHUK_B:
    {
        istringstream istr(_buf);
        int len;
        istr >> hex  >> len;
        len += 2; // 2 is chunk tail



        if (_spFrame == 0)
        {
            JIF(m_spAllocate->Alloc(&_spFrame));
            _spFrame->SetBuf(0,len);
        }
        _spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
        const IMediaFrame::buf* buf = _spFrame->GetBuf();
        JIF(m_spStream->Read(buf->data, buf->size));
        ((IMediaFrame::buf*)buf)->size -= 2;
        hr = m_demuxer->OnEvent(_spFrame.p, it, type, param1, param2, param3);
        //        std::cout << "chunked data len :" << buf->size <<endl;
        _spFrame = NULL;
        _status = RECV_CHUK_H;
        _buf.clear();
        return hr;
    }
    case RECV_CONTENT:
    {
        hr = m_demuxer->OnEvent(m_spStream.p,it, ET_Stream_Read, _contentLen - (m_spStream->GetStatus().read_total_size - _alreadyRead), param2, param3);
        if(false == _isRes)
        {
            if(S_OK  > hr && E_AGAIN != hr)
            {
                string header;
                if(E_EOF == hr)
                    header = get_response_header(200,m_url.m_format.c_str());
                else
                    header = get_response_header(600,m_url.m_format.c_str());
                JIF(m_spStream->Write(header.c_str(),header.size(),IStream::WRITE_FLAG_EOF));
            }
        }
        return hr;
    }
    case RECV_DIRECT2:
    {
        hr = m_demuxer->OnEvent(m_spStream.p,it,type,param1,param2,param3);
        if(false == _isRes)
        {
            if(S_OK  > hr && E_AGAIN != hr)
            {
                string header;
                if(E_EOF == hr)
                    header = get_response_header(200,m_url.m_format.c_str());
                else
                    header = get_response_header(600,m_url.m_format.c_str());
                JIF(m_spStream->Write(header.c_str(),header.size(),IStream::WRITE_FLAG_EOF));
            }
        }
        return hr;
    }
    default:
    {
        LOG(0, "invalid status, reset to uninit");
        _status = RECV_HTTP_H;
    }
    }
    return hr;
}


HRESULT CHttpSession::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(STR_CMP(protocol,HTTP_PROTOCOL_NAME))
        return 1;
    return E_INVALIDARG;
}

HRESULT CHttpSession::CreateListener(IStreamListen** ppListen,Interface* pObj,uint16_t* pPort)
{
    if(NULL != ppListen)
    {
        dom_ptr<IStreamListen> spListen;
        JCHK(spListen.Create(CLSID_CNetworkListen,NULL,false,pObj),E_FAIL);
        spListen.CopyTo(ppListen);
    }
    if(NULL != pPort)
    {
        *pPort = HTTP_PORT;
    }
    return 1;
}

HRESULT CHttpSession::send_404()
{
    HttpHeader h;
    h.setConnectionClose();
    h.setContentType("ts");
    h.addValue("Progma", "no-cache");
    string s = h.getResponseString(404);
    return m_spStream->Write(s.c_str(),s.size());
}

string CHttpSession::get_response_header(int sc, const char* tp, uint32_t len)
{
    HttpHeader h;
    h.setConnectionClose();
    h.addValue("Progma", "no-cache");
    h.setContentType(tp);
    h.addValue("Access-Control-Allow-Origin", "*");
    h.addValue("Accept-Ranges", "bytes");

    if (0 < len)
    {
        h.setContentLength(len);
        string s = "bytes 0-";
        s += boost::lexical_cast<string>(len-1);
        s += "/";
        s += boost::lexical_cast<string>(len);
        h.addValue("Content-Range", s);
    }
    return h.getResponseString(sc);
}
