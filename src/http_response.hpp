#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "http_request.hpp"
#include "common.hpp"
#include <zlib.h>
#include <string>
#include <map>

enum t_response_answer
{
    RT_OK = 0,
    RT_CREATED,
    RT_NOT_FOUND,
    RT_SERVER_ERROR
};

std::string to_string(t_response_answer type);

class HttpResponse
{
public:
    HttpResponse(t_response_answer type, t_http_version version) : m_type(type), m_version(version)
    {
    }
    std::string str()
    {
        PrepareBody();
        std::stringstream ss;
        ss << to_string(m_version) << ' ' << to_string(m_type) << "\r\n";
        for (const auto& line : m_headers)
        {
            ss << line.first << ": " << line.second << "\r\n";
        }
        ss << "\r\n";
        if (!m_body.empty())
        {
            ss << m_body;
        }
        return ss.str();
    }
    void SetContentLength(size_t length)
    {
        m_headers["Content-Length"] = std::to_string(length);
    }
    void SetContentType(const std::string& type)
    {
        m_headers["Content-Type"] = type;
    }
    void SetEncoding(const std::string& encoding)
    {
        m_headers["Content-Encoding"] = encoding;
    }
    void SetBody(const std::string& body)
    {
        m_body = body;
    }
private:
    void PrepareBody()
    {
        if (m_headers.find("Content-Encoding") != m_headers.end() && !m_body.empty())
        {
            m_body = compress(m_body);
            SetContentLength(m_body.size());
        }
    }

private:
    t_response_answer m_type;
    t_http_version m_version;
    std::map<std::string, std::string> m_headers;
    std::string m_body;
};

#endif
