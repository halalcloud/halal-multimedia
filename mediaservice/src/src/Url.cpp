#ifndef URL_CPP
#define URL_CPP

#include <stdio.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <arpa/inet.h>
#include <dump.h>
#include "../inc/iMulitimedia.h"

using namespace std;

const char HOST_NAME[] = "vhost";
const char STREAM_NAME[] = "vstream";
const char STREAM_SEEK[] = "seek";
const char STREAM_DURATION[] = "duration";

class CUrl
{
public:
    typedef map< string,string > ParamSet;
    typedef ParamSet::iterator ParamIt;
    typedef pair<ParamSet::key_type,ParamSet::mapped_type> ParamPair;
    CUrl():m_port(0),m_pos(MEDIA_FRAME_NONE_TIMESTAMP),m_duration(0){}
    CUrl(CUrl& url)
    {
        m_protocol = url.m_protocol;
        m_host = url.m_host;
        m_port = url.m_port;
        m_path = url.m_path;
        m_file = url.m_file;
        m_format = url.m_format;
        m_pos = url.m_pos;
        m_duration = url.m_duration;
        ParseParams(url.GetParames());
    }
    CUrl(const char* pUrl):m_port(0),m_pos(MEDIA_FRAME_NONE_TIMESTAMP),m_duration(0)
    {
        if(NULL != pUrl)
            Set(pUrl);
    }
    HRESULT Set(const char* pUrl)
    {
        JCHK(pUrl,E_INVALIDARG);

        string params;
        string url = pUrl;
        transform(url.begin(), url.end(), url.begin(),::tolower);

        m_protocol.clear();
        m_host.clear();
        m_port = 0;
        m_path.clear();
        m_file.clear();
        m_format.clear();
        m_pos = MEDIA_FRAME_NONE_TIMESTAMP;
        m_duration = 0;

        size_t pos = 0;
        size_t end = url.find("://");
        if(string::npos != end)
        {
            m_protocol = url.substr(pos,end);
            pos = end + 3;
            end = url.find('/',pos);
            string host;
            if(string::npos != end)
                host = url.substr(pos,end-pos);
            else
                host = url.substr(pos);
            size_t mid = host.find_last_of(':');
            if(string::npos != mid)
            {
                m_host = host.substr(0,mid++);
                string port_str = host.substr(mid);
                m_port = (uint16_t)atoi(port_str.c_str());
            }
            else
            {
                m_host = host;
            }
            if(string::npos != end)
            {
                pos = end;
                end = url.find('?',pos);

                string str;
                if(string::npos != end)
                    str = url.substr(pos,end-pos);
                else
                    str= url.substr(pos);

                pos = str.find_last_of('/');
                m_path = str.substr(0,++pos);

                str = str.substr(pos);

                pos = str.find_last_of('.');

                if(string::npos != pos)
                {
                    m_file = str.substr(0,pos);
                    m_format = str.substr(pos+1);
                }
                else
                    m_file = str;
            }
            else
            {
                pos = end;
                end = url.find('?',pos);
            }
            if(string::npos != end)
                params = url.substr(end+1);
        }
        else
        {
            m_protocol.clear();
            m_host.clear();

            pos = url.find_last_of('?');
            if(string::npos != pos)
            {
                params = url.substr(pos+1);
                url = url.substr(0,pos);
            }
            else
                params.clear();

            pos = url.find_last_of('/');

            if(string::npos != pos)
            {
                m_path = url.substr(0,++pos);
                url = url.substr(pos);
            }
            else
                m_path.clear();

            pos = url.find_last_of('.');
            if(string::npos != pos)
            {
                m_file = url.substr(0,pos);
                m_format = url.substr(pos+1);
            }
            else
            {
                m_file = url;
                m_format.clear();
            }
        }

        ParseParams(params);

        m_url = url;

        return S_OK;
    }
	const char* Get(bool param = true)
	{
        m_url.clear();

        if(false == m_protocol.empty())
        {
            m_url = m_protocol + "://" + m_host;
            if(0 != m_port)
            {
                char str[10];
                snprintf(str,10,"%u",m_port);
                m_url += ":";
                m_url += str;
            }
        }

        if(false == m_path.empty())
            m_url += m_path;

        if(false == m_file.empty())
            m_url += m_file;

        if(false == m_format.empty())
        {
            m_url += '.';
            m_url += m_format;
        }

        if(true == param)
        {
            string params = GetParames();
            if(false == params.empty())
            {
                m_url += '?';
                m_url += params;
            }
        }

        transform(m_url.begin(), m_url.end(), m_url.begin(), ::tolower);
        return m_url.c_str();
	}

	HRESULT SetStreamID(const char* pProtocol,const char* pName)
	{
        JCHK(NULL != pName,E_INVALIDARG);

        if(NULL != pProtocol)
            m_protocol = pProtocol;

        string name = pName;
        transform(name.begin(), name.end(), name.begin(), ::tolower);

        size_t pos = name.find('?');

        m_setParam.clear();
        if(pos != string::npos)
        {
            string params = name.substr(pos+1);
            ParseParams(params);
            name = name.substr(0,pos);
        }

        pos = name.find('/');
        if(pos != string::npos)
        {
            m_host = name.substr(0,pos);

            name = name.substr(pos);
            pos = name.find_last_of('/');

            m_path = name.substr(0,++pos);

            name = name.substr(pos);
            pos = name.find('.');
            if(string::npos != pos)
            {
                m_file = name.substr(0,pos);
                m_format = name.substr(pos+1);
            }
            else
            {
                m_file = name;
                m_format.clear();
            }
        }
        else
        {
            m_host = name;
            m_path.clear();
            m_file.clear();
            m_format.clear();
        }
        return S_OK;
	}

	string GetStreamID(const char* pFormat = NULL,bool param = true)
	{
	    string result;
	    ParamIt it;

        it = m_setParam.find(HOST_NAME);
        if(it != m_setParam.end())
        {
            result = it->second;
            m_setParam.erase(it);
        }
        else
        {
            if(true == m_host.empty() || INADDR_NONE != inet_addr(m_host.c_str()))
                result = HOST_NAME;
            else
                result = m_host;
        }
        if(false == m_path.empty())
            result += m_path;

        it = m_setParam.find(STREAM_NAME);
        if(it != m_setParam.end())
        {
            result += it->second;
            m_setParam.erase(it);
        }
        else
        {
            if(false == m_file.empty())
                result += m_file;
            if(pFormat != NULL)
            {
                result += '.';
                result += pFormat;
            }
            else if(false == m_format.empty())
            {
                result += '.';
                result += m_format;
            }
        }
        if(true == param)
        {
            string params = GetParames();
            if(false == params.empty())
            {
                result += '?';
                result += params;
            }
        }
        return result;
	}
	string GetStreamName()
	{
	    string result;
        result = m_file;
        if(false == m_format.empty())
        {
            result += '.';
            result += m_format;
        }
        return result;
	}
	string GetPath(const char* root = NULL)
	{
	    string result;
	    if(NULL != root)
            result = root;
        if(false == m_host.empty())
        {
            result += '/';
            result += m_host;
        }
        result += m_path;
        result += m_file;
        if(false == m_format.empty())
        {
            result += '.';
            result += m_format;
        }
        return result;
	}
	void ParseParams(const string& params)
	{
        if(false == params.empty())
        {
            string item;
            size_t beg = 0,end;
            while(string::npos != (end = params.find('&',beg)))
            {
                item = params.substr(beg,end-beg);
                if(false == item.empty())
                {
                    size_t equal;
                    if(string::npos != (equal = item.find('=')))
                        m_setParam.insert(ParamPair(item.substr(0,equal),item.substr(equal+1)));
                }
                beg = end + 1;
            }
            item = params.substr(beg);
            if(false == item.empty())
            {
                size_t equal;
                if(string::npos != (equal = item.find('=')))
                    m_setParam.insert(ParamPair(item.substr(0,equal),item.substr(equal+1)));
            }
        }
        ParseSeek();
	}
	string GetParames()
	{
	    string result;
        if(false == m_setParam.empty())
        {
            ParamIt it = m_setParam.begin();
            while(it != m_setParam.end())
            {
                if(it != m_setParam.begin())
                    result += '&';
                result += it->first;
                result += '=';
                result += it->second;
                ++it;
            }
        }
        return result;
	}

    void ParseSeek()
    {
        ParamIt itSeek = ParsePos(STREAM_SEEK,m_pos);
        ParamIt itDuration = ParsePos(STREAM_DURATION,m_duration);

        if(m_setParam.end() != itSeek)
        {
            if(0 <= m_pos)
            {
                char str[20];
                snprintf(str,20,"%ld",m_pos);
                itSeek->second = str;
            }
            else
            {
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                int64_t ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
                m_pos = ms + m_pos;
            }
        }
        else
            m_pos = MEDIA_FRAME_NONE_TIMESTAMP;

        if(m_setParam.end() != itDuration)
        {
            if(m_duration > 0)
            {
                char str[20];
                snprintf(str,20,"%ld",m_duration);
                itDuration->second = str;
            }
            else
            {
                m_duration = 0;
                m_setParam.erase(itDuration);
            }
        }
        else
            m_duration = 0;
    }

    ParamIt ParsePos(const char* Key,int64_t& pos)
	{
	    pos = MEDIA_FRAME_NONE_TIMESTAMP;
        ParamIt it = m_setParam.find(Key);
        if(it != m_setParam.end())
        {
            char* pBeg;
            char* pEnd;
            tm tm_;
            pBeg = (char*)it->second.c_str();
            pEnd = strptime(pBeg,"%Y-%m-%d_%H:%M:%S",&tm_);
            if(pEnd > pBeg)
            {
                pos = mktime(&tm_) * 1000;

                if(*pEnd == '_')
                {
                    pBeg = pEnd;
                    int64_t ms = strtoll(pBeg,&pEnd,10);
                    if(pEnd > pBeg && 0 < ms && 1000 > ms)
                        pos += ms;
                }
            }
            else
            {
                pEnd = strptime(pBeg,"-%H:%M:%S",&tm_);
                if(pEnd > pBeg)
                {
                    pos = time(NULL) * 1000;
                    pos -= (tm_.tm_hour * 3600 + tm_.tm_min * 60 + tm_.tm_sec) * 1000;
                    if(*pEnd == '_')
                    {
                        pBeg = pEnd;
                        int64_t ms = strtoll(pBeg,&pEnd,10);
                        if(pEnd > pBeg && 0 < ms && 1000 > ms)
                            pos -= ms;
                    }
                }
                else
                {
                    pEnd = strptime(pBeg,"%H:%M:%S",&tm_);
                    if(pEnd > pBeg)
                    {
                        pos = (tm_.tm_hour * 3600 + tm_.tm_min * 60 + tm_.tm_sec)*1000;
                        if(*pEnd == '_')
                        {
                            pBeg = pEnd;
                            int64_t ms = strtoll(pBeg,&pEnd,10);
                            if(pEnd > pBeg && 0 < ms && 1000 > ms)
                                pos += ms;
                        }
                    }
                    else
                        pos = strtoll(pBeg,&pEnd,10);
                }
            }
        }
        return it;
	}
public:
    string m_protocol;
    string m_host;
    uint16_t m_port;
    string m_path;
    string m_file;
    string m_format;
    string m_url;
    ParamSet m_setParam;
    int64_t m_pos;
    int64_t m_duration;
};

#endif // URL_CPP
