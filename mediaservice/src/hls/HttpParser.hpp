#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "http_parser.h"
#include <string>
#include <map>

// for http parser macros
#define LMS_HTTP_METHOD_OPTIONS      HTTP_OPTIONS
#define LMS_HTTP_METHOD_GET          HTTP_GET
#define LMS_HTTP_METHOD_POST         HTTP_POST
#define LMS_HTTP_METHOD_PUT          HTTP_PUT
#define LMS_HTTP_METHOD_DELETE       HTTP_DELETE

class HttpParser
{
	// functions start with _ should not be called in outside.
    static int onMessageBegin(http_parser* parser);
    static int onHeaderscComplete(http_parser* parser);
    static int onMessageComplete(http_parser* parser);
    static int onUrl(http_parser* parser, const char* at, size_t length);
    static int onHeaderField(http_parser* parser, const char* at, size_t length);
    static int onHeaderValue(http_parser* parser, const char* at, size_t length);
    static int onBody(http_parser* parser, const char* at, size_t length);
public:
    // enum http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH };
    HttpParser(enum http_parser_type type);
    ~HttpParser();

    bool isRequest()
    {
        return m_parser.type == HTTP_REQUEST?true:false;
    }
    int parse(const char *data, unsigned int length);
	/**
	 *	下列函数只有parse成功之后才能调用。
	 */
    bool completed();
    std::string getUrl();
	/**
	 * @return 成功返回Key对应的内容，失败返回空串
	 */
    std::string feild(const std::string &key);
    std::string method();
    uint16_t statusCode();
private:
    void init(enum http_parser_type type);

private:
    http_parser_settings m_settings;
    http_parser m_parser;

    bool m_completed;
    std::string m_url;

    bool m_lasWasValue;
    std::string m_field_key;
    std::string m_field_value;
    std::map<std::string, std::string> m_fields;
};

#endif // HttpParser_HPP
