#include "DHttpHeader.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sstream>
using namespace std;
std::map<string, string> HttpHeader::m_mime;

HttpHeader::HttpHeader()
{
    if (m_mime.empty()) {
        m_mime["html"]      = "text/html";
        m_mime["htm"]       = "text/html";
        m_mime["shtml"]     = "text/html";
        m_mime["css"]       = "text/css";
        m_mime["xml"]       = "text/xml";
        m_mime["gif"]       = "image/gif";
        m_mime["jpeg"]      = "image/jpeg";
        m_mime["jpg"]       = "image/jpeg";
        m_mime["js"]        = "application/javascript";
        m_mime["atom"]      = "application/atom+xml";
        m_mime["rss"]       = "application/rss+xml";

        m_mime["mml"]       = "text/mathml";
        m_mime["txt"]       = "text/plain";
        m_mime["jad"]       = "text/vnd.sun.j2me.app-descriptor";
        m_mime["wml"]       = "text/vnd.wap.wml";
        m_mime["htc"]       = "text/x-component";

        m_mime["png"]       = "image/png";
        m_mime["tif"]       = "image/tiff";
        m_mime["tiff"]      = "image/vnd.wap.wbmp";
        m_mime["wbmp"]      = "image/x-icon";
        m_mime["ico"]       = "image/x-jng";
        m_mime["jng"]       = "image/x-ms-bmp";
        m_mime["bmp"]       = "text/x-component";
        m_mime["svg"]       = "image/svg+xml";
        m_mime["svgz"]      = "image/svg+xml";
        m_mime["webp"]      = "image/webp";

        m_mime["woff"]      = "application/font-woff";
        m_mime["jar"]       = "application/java-archive";
        m_mime["war"]       = "application/java-archive";
        m_mime["ear"]       = "application/java-archive";
        m_mime["json"]      = "application/json";
        m_mime["hqx"]       = "application/mac-binhex40";
        m_mime["doc"]       = "application/msword";
        m_mime["pdf"]       = "application/pdf";
        m_mime["ps"]        = "application/postscript";
        m_mime["eps"]       = "application/postscript";
        m_mime["ai"]        = "application/postscript";


        m_mime["rtf"]       = "application/rtf";
        m_mime["xls"]       = "application/vnd.ms-excel";
        m_mime["eot"]       = "application/vnd.ms-fontobject";
        m_mime["ppt"]       = "application/vnd.ms-powerpoint";
        m_mime["wmlc"]      = "application/vnd.wap.wmlc";
        m_mime["kml"]       = "application/vnd.google-earth.kml+xml";
        m_mime["kmz"]       = "application/vnd.google-earth.kmz";
        m_mime["7z"]        = "application/x-7z-compressed";
        m_mime["cco"]       = "application/x-cocoa";
        m_mime["jardiff"]   = "application/x-java-archive-diff";

        m_mime["jnlp"]      = "application/x-java-jnlp-file";
        m_mime["run"]       = "application/x-makeself";
        m_mime["pl"]        = "application/x-perl";
        m_mime["pm"]        = "application/x-perl";
        m_mime["prc"]       = "application/x-pilot";
        m_mime["pdb"]       = "application/x-pilot";
        m_mime["rar"]       = "application/x-rar-compressed";
        m_mime["rpm"]       = "application/x-redhat-package-manager";
        m_mime["sea"]       = "application/x-sea";
        m_mime["swf"]       = "application/x-shockwave-flash";


        m_mime["sit"]       = "application/x-stuffit";
        m_mime["tcl"]       = "application/x-tcl";
        m_mime["tk"]        = "application/x-tcl";
        m_mime["der"]       = "application/x-x509-ca-certl";
        m_mime["pem"]       = "application/x-x509-ca-cert";
        m_mime["crt"]       = "application/x-x509-ca-cert";
        m_mime["xpi"]       = "application/x-xpinstall";
        m_mime["xhtml"]     = "application/xhtml+xml";
        m_mime["zip"]       = "application/zip";

        m_mime["bin"]       = "application/octet-stream";
        m_mime["exe"]       = "application/octet-stream";
        m_mime["dll"]       = "application/octet-stream";
        m_mime["deb"]       = "application/octet-stream";
        m_mime["dmg"]       = "application/octet-stream";
        m_mime["iso"]       = "application/octet-stream";
        m_mime["img"]       = "application/octet-stream";
        m_mime["msi"]       = "application/octet-stream";
        m_mime["msp"]       = "application/octet-stream";
        m_mime["msm"]       = "application/octet-stream";

        m_mime["docx"]      = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        m_mime["xlsx"]      = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        m_mime["pptx"]      = "application/vnd.openxmlformats-officedocument.presentationml.presentation";

        m_mime["mid"]       = "audio/midi";
        m_mime["midi"]      = "audio/midi";
        m_mime["kar"]       = "audio/midi";
        m_mime["mp3"]       = "audio/mpeg";
        m_mime["ogg"]       = "audio/ogg";
        m_mime["m4a"]       = "audio/x-m4a ";
        m_mime["ra"]        = "audio/x-realaudio";
        m_mime["3gpp"]      = "video/3gpp";
        m_mime["3gp"]       = "video/3gpp";
        m_mime["mp4"]       = "video/mp4";
        m_mime["mpeg"]      = "video/mpeg";
        m_mime["mpg"]       = "video/mpeg";
        m_mime["mov"]       = "video/quicktime";
        m_mime["webm"]      = "video/webm";
        m_mime["flv"]       = "video/x-flv";
        m_mime["m4v"]       = "video/x-m4v";
        m_mime["mng"]       = "video/x-mng";
        m_mime["asx"]       = "video/x-ms-asf";
        m_mime["asf"]       = "video/x-ms-asf";
        m_mime["wmv"]       = "video/x-ms-wmv";
        m_mime["avi"]       = "video/x-msvideo";
        m_mime["ts"]        = "video/MP2T";
        m_mime["m3u8"]      = "application/x-mpegURL";

        m_mime["default"]   = "application/octet-stream";
    }
}

HttpHeader::~HttpHeader()
{

}

void HttpHeader::addValue(const string &key, const string &value)
{
    if (key.empty() || value.empty()) {
        return;
    }

    m_headers[key] = value;
}

void HttpHeader::setContentLength(int len)
{
    ostringstream ostr;
    ostr << len;
    addValue("Content-Length", ostr.str());
}

void HttpHeader::setContentType(const string &name)
{
    string type = getContentType(name);
    addValue("Content-Type", type);
}

void HttpHeader::setHost(const string &host)
{
    addValue("Host", host);
}

void HttpHeader::setServer(const string &server)
{
    addValue("Server", server);
}

void HttpHeader::setConnectionClose()
{
    addValue("Connection", "close");
}

void HttpHeader::setConnectionKeepAlive()
{
    addValue("Connection", "Keep-Alive");
}

void HttpHeader::setDate()
{
    string date = generateDate();
    if (!date.empty()) {
        addValue("Date", date);
    }
}

string HttpHeader::getRequestString(const string &type, const string &url)
{
    string ret;
    std::map<string, string>::iterator iter;

    ret.append(type).append(" ").append(url).append(" ").append("HTTP/1.1").append("\r\n");

    for (iter = m_headers.begin(); iter != m_headers.end(); ++iter) {
        string key = iter->first;
        string value = iter->second;
        ret.append(key).append(": ").append(value).append("\r\n");
    }
    ret.append("\r\n");

    return ret;
}

string HttpHeader::getResponseString(int code)
{
    string ret;
    std::map<string, string>::iterator iter;

    string status = getStatusInfo(code);
ostringstream ostr;
ostr << code;
    ret.append("HTTP/1.1 ").append(ostr.str()).append(" ").append(status).append("\r\n");

    for (iter = m_headers.begin(); iter != m_headers.end(); ++iter) {
        string key = iter->first;
        string value = iter->second;
        ret.append(key).append(": ").append(value).append("\r\n");
    }
    ret.append("\r\n");

    return ret;
}

string HttpHeader::getContentType(const string &fileType)
{
    if (m_mime.find(fileType) != m_mime.end()) {
        return m_mime[fileType];
    }

    return m_mime["default"];
}

string HttpHeader::generateDate()
{
    char localtm[80];
    time_t now;
    struct tm timenow = {0};
    time(&now);

    localtime_r(&now, &timenow);

    char daytime[50];
    asctime_r(&timenow, daytime);

    char *pch = NULL, *week = NULL, *mon = NULL, *day = NULL, *time = NULL, *year = NULL;
    pch = strtok(daytime, " ");

    int i = 0;
    while(pch != NULL)
    {
        switch(i)
        {
        case 0:
            week = pch;
            break;
        case 1:
            mon = pch;
            break;
        case 2:
            day = pch;
            break;
        case 3:
            time = pch;
            break;
        case 4:
            year = pch;
            break;
        default:
            break;
        }
        pch = strtok(NULL, " ");
        i++;
    }


    if (!week || !mon || !day || !time || !year) {
        return "";
    }

    char realyear[4];
    strncpy(realyear, year, 4);

    sprintf(localtm, "%s, %02d %s %s %s GMT", week, atoi(day), mon, realyear, time);

    return string(localtm);
}

string HttpHeader::getStatusInfo(int code)
{
    string info;

    switch (code) {
    case 200:
        info = "OK";
        break;
    case 206:
        info = "Partial Content";
        break;
    case 301:
        info = "Moved Permanently";
        break;
    case 302:
        info = "Found";
        break;
    case 403:
        info = "Forbidden";
        break;
    case 404:
        info = "Not Found";
        break;

        break;
    default:
        break;
    }

    return info;
}
