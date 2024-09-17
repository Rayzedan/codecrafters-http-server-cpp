#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "common.hpp"
#include <map>
#include <string>
#include <sstream>
#include <string>
#include <iostream>

enum class t_request_type
{
    RT_GET = 0,
    RT_POST,
    RT_UNKNOWN
};

t_request_type get_request_type(const std::string& request);
std::string to_string(t_request_type type);

class RequestStatus
{
public:
    RequestStatus() = default;
    RequestStatus(const std::string& parsed_request)
    {
        std::istringstream ss(parsed_request);
        std::string rawMethod;
        ss >> rawMethod;
        m_method = get_request_type(rawMethod);
        std::string rawTarget;
        ss >> rawTarget;
        m_target = split(rawTarget);
        ss >> m_version;
    }
    t_request_type GetMethod() const
    {
        return m_method;
    }
    std::vector<std::string> GetTarget() const
    {
        return m_target;
    }
    std::string GetVersion() const
    {
        return m_version;
    }
    friend std::ostream& operator<<(std::ostream& os, RequestStatus requestLine)
    {
        os << "Method: " << to_string(requestLine.m_method) << std::endl;
        os << "Target: ";
        for (const auto& target : requestLine.m_target)
        {
            os << target;
        }
        os << std::endl;
        os << "Version: " << requestLine.m_version << std::endl;
        return os;
    }
private:
    t_request_type m_method;
    std::vector<std::string> m_target;
    std::string m_version;
};

class HttpRequest {
public:
    HttpRequest(const std::string& rawRequest)
    {
        if (!rawRequest.empty())
        {
            const auto idx = rawRequest.find_first_of("\r\n");
            if (idx == std::string::npos)
            {
                throw std::runtime_error("Error while parsing HTTP method");
            }
            m_request_line = RequestStatus(rawRequest.substr(0, idx));
            m_headers = ParseHeaders(rawRequest.substr(idx + 2));
            auto end = rawRequest.find_last_of("\r\n\n");
            if (end != std::string::npos)
            {
                m_body = rawRequest.substr(end + 1);
            }
        }
    }
    RequestStatus GetStatus() const
    {
        return m_request_line;
    }
    std::map<std::string, std::string> GetHeaders() const
    {
        return m_headers;
    }
    std::string GetBody() const
    {
        return m_body;
    }
    friend std::ostream& operator<<(std::ostream& os, const HttpRequest& request)
    {
        os << request.m_request_line;
        for (const auto& header : request.m_headers)
        {
            os << header.first << ": " << header.second << std::endl;
        }
        return os;
    }
private:
    std::map<std::string, std::string> ParseHeaders(const std::string& headers)
    {
        std::map<std::string, std::string> headers_map;
        size_t start = 0;
        size_t end;
        while ((end = headers.find("\r\n", start)) != std::string::npos)
        {
            std::string header = headers.substr(start, end - start);
            if (header.empty())
            {
                break;
            }
            size_t colon_idx = header.find(':');
            if (colon_idx == std::string::npos)
            {
                throw std::runtime_error("Invalid header format: no colon found");
            }
            std::string key = header.substr(0, colon_idx);
            std::string value = header.substr(colon_idx + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            headers_map[key] = value;
            start = end + 2;
        }
        return headers_map;
    }

private:
    RequestStatus m_request_line;
    std::map<std::string, std::string> m_headers;
    std::string m_body;
};

#endif
