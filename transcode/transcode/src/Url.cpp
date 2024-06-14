#include <ctype.h>
#include <string>
#include <iostream>
#include <algorithm>
#include "../inc/url.h"

using namespace std;

class CUrl : public url
{
public:
    CUrl();
    CUrl(const string& url){Set(url);}
    void Set(const string& url)
    {
        size_t pos = 0;
        size_t end = url.find("://");
        if(string::npos != end)
        {
            m_protocol = url.substr(pos,end);
            transform(m_protocol.begin(), m_protocol.end(), m_protocol.begin(),::tolower);
            pos = end + 3;
            end = url.find('/',pos);

            string host = url.substr(pos,end-pos);
            size_t mid = host.find_last_of(':');
            if(string::npos != mid)
            {
                m_host = host.substr(0,mid++);
                m_port = host.substr(mid);
            }
            else
            {
                m_host = host;
                m_port.clear();
            }
            transform(m_host.begin(), m_host.end(), m_host.begin(), ::tolower);
            transform(m_port.begin(), m_port.end(), m_port.begin(), ::tolower);
            if(string::npos != end)
            {
                pos = end;
                end = url.find('?',pos);
                if(string::npos != end)
                    m_path = url.substr(pos,end-pos);
                else
                    m_path = url.substr(pos);
                pos = m_path.find_last_of('/');
                m_file = m_path.substr(pos+1);
                pos = m_file.find_last_of('.');
                if(string::npos != pos)
                {
                    m_format = m_file.substr(pos+1);
                    transform(m_format.begin(), m_format.end(), m_format.begin(), ::tolower);
                }
                else
                    m_format.clear();
            }
            if(string::npos != end)
            {
                m_params = url.substr(end);
                transform(m_params.begin(), m_params.end(), m_params.begin(), ::tolower);
            }
            else
                m_params.clear();
        }
        else
        {
            m_protocol.clear();
            m_host.clear();
            m_port.clear();
            m_path = url;
            pos = m_path.find_last_of('/');
            if(string::npos != pos)
            {
                m_file = m_path.substr(pos+1);
                pos = m_file.find_last_of('.');
                if(string::npos != pos)
                {
                    m_format = m_file.substr(pos+1);
                    transform(m_format.begin(), m_format.end(), m_format.begin(), ::tolower);
                }
                else
                    m_format.clear();
            }
            else
                m_file.clear();
            m_params.clear();
        }
        m_url = url;
        protocol = m_protocol.empty() ? NULL : m_protocol.c_str();
        host = m_host.empty() ? NULL : m_host.c_str();
        port = m_port.empty() ? NULL : m_port.c_str();
        path = m_path.empty() ? NULL : m_path.c_str();
        file = m_file.empty() ? NULL : m_file.c_str();
        format = m_format.empty() ? NULL : m_format.c_str();
        params = m_params.empty() ? NULL : m_params.c_str();
    }
	const char* Get()
	{
        if(m_url.empty())
        {
            m_protocol = protocol;
            m_host = host;
            m_port = port;
            m_path = path;
            m_file = file;
            m_format = format;
            m_params = params;
            if(false == m_protocol.empty())
            {
                m_url = m_protocol + "://" + m_host;
                if(false == m_port.empty())
                    m_url += ":" + m_port;
                m_url += m_path;
                if(false == m_params.empty())
                    m_url += m_params;
            }
            else
                m_url = m_path;
        }
        return m_url.c_str();
	}
protected:
   string m_protocol;
   string m_host;
   string m_port;
   string m_path;
   string m_file;
   string m_format;
   string m_params;
   string m_url;
};
