#ifndef DHTTPHEADER_HPP
#define DHTTPHEADER_HPP

#include <map>
#include <string>

class HttpHeader
{
public:
    HttpHeader();
    ~HttpHeader();

    void addValue(const std::string &key, const std::string &value);

    void setContentLength(int len);
    void setContentType(const std::string &name);
	/**
	 * @param[in]格式为 '地址:端口'，若端口为80可直接写为‘地址’
	 */
    void setHost(const std::string &host);
    void setServer(const std::string &server);
	/**
	 * @note 此函数和setConnectionKeepAlive互斥。由调用者保证。
	 */
    void setConnectionClose();
	/**
	 * @note 此函数和setConnectionKeepAlive互斥。由调用者保证。
	 */
    void setConnectionKeepAlive();
    void setDate();

    /**
     * @brief getRequestString
     * @param type GET/POST
     * @param url  /test/1.flv
     */
    std::string getRequestString(const std::string &type, const std::string &url);
    /**
     * @brief getResponseString
     * @param err 200/302
     * @param info OK/Found
     */
    std::string getResponseString(int code);
private:
    std::string generateDate();
    std::string getContentType(const std::string &fileType);
	    std::string getStatusInfo(int code);
private:
    std::map<std::string, std::string> m_headers;

    static std::map<std::string, std::string> m_mime;
};

#endif // DHTTPHEADER_HPP
