#include "HttpParser.hpp"
#include <string>
#include <memory>
#include <string.h>
HttpParser::HttpParser( http_parser_type type)
{
    m_lasWasValue = true;
    init(type);
}

HttpParser::~HttpParser()
{

}

void HttpParser::init(enum http_parser_type type)
{
    memset(&m_settings, 0, sizeof(m_settings));
    m_settings.on_message_begin = onMessageBegin;
    m_settings.on_url = onUrl;
    m_settings.on_header_field = onHeaderField;
    m_settings.on_header_value = onHeaderValue;
    m_settings.on_headers_complete = onHeaderscComplete;
    m_settings.on_body = onBody;
    m_settings.on_message_complete = onMessageComplete;

    http_parser_init(&m_parser, type);
    m_parser.data = reinterpret_cast<void*>(this);
}

int HttpParser::parse(const char *data, unsigned int length)
{
    int nparsed = http_parser_execute(&m_parser, &m_settings, data, length);

    if (m_parser.http_errno != 0) {
        return -1;
    }

    return nparsed;
}

bool HttpParser::completed()
{
    return m_completed;
}

std::string HttpParser::getUrl()
{
    return m_url;
}

std::string HttpParser::feild(const std::string &key)
{
    std::map<std::string, std::string>::iterator iter;
    iter = m_fields.find(key);
    if (iter != m_fields.end())
    {
        return m_fields[key];
    }

    return "";
}

std::string HttpParser::method()
{
    return http_method_str((enum http_method)m_parser.method);
}

uint16_t HttpParser::statusCode()
{
    return m_parser.status_code;
}

int HttpParser::onMessageBegin(http_parser *parser)
{
    HttpParser *obj = (HttpParser*)parser->data;
    obj->m_completed = false;
    obj->m_lasWasValue = true;

    return 0;
}

int HttpParser::onHeaderscComplete(http_parser *parser)
{
    HttpParser *obj = (HttpParser*)parser->data;
    obj->m_completed = true;

    if (!obj->m_field_key.empty()) {
        obj->m_fields[obj->m_field_key] = obj->m_field_value;
    }

    return 0;
}

int HttpParser::onMessageComplete(http_parser *parser)
{
    return 0;
}

int HttpParser::onUrl(http_parser *parser, const char *at, size_t length)
{
    HttpParser *obj = (HttpParser*)parser->data;
    if (length > 0) {
        obj->m_url = std::string(at, (int)length);
    }

    return 0;
}

int HttpParser::onHeaderField(http_parser *parser, const char *at, size_t length)
{
    HttpParser *obj = (HttpParser*)parser->data;

    if (obj->m_lasWasValue) {
        if (!obj->m_field_key.empty()) {
            obj->m_fields[obj->m_field_key] = obj->m_field_value;
        }

        obj->m_field_key.clear();
        obj->m_field_value.clear();
    }

    obj->m_field_key.append(at, length);
    obj->m_lasWasValue = false;

    return 0;
}

int HttpParser::onHeaderValue(http_parser *parser, const char *at, size_t length)
{
    HttpParser *obj = (HttpParser*)parser->data;
    obj->m_field_value.append(at, length);
    obj->m_lasWasValue = true;

    return 0;
}

int HttpParser::onBody(http_parser *parser, const char *at, size_t length)
{
    return 0;
}
