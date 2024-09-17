#include "http_request.hpp"
#include <string>

t_request_type get_request_type(const std::string& request)
{
    if (request.find("POST") != std::string::npos)
    {
        return t_request_type::RT_POST;
    }
    if (request.find("GET") != std::string::npos)
    {
        return t_request_type::RT_GET;
    }
    throw std::invalid_argument("Invalid request type");
}


std::string to_string(t_request_type type)
{
    switch (type)
    {
        case t_request_type::RT_GET:
            return "GET";
        case t_request_type::RT_POST:
            return "POST";
        default:
            return "UNKNOWN";
    }
}

t_http_version get_version(const std::string& version)
{
    if (version == "HTTP/1.0")
    {
        return t_http_version::HV_1_0;
    }
    if (version == "HTTP/1.1")
    {
        return t_http_version::HV_1_1;
    }
    throw std::invalid_argument("Invalid version");
}

std::string to_string(t_http_version type)
{
    switch (type)
    {
        case t_http_version::HV_1_0:
            return "HTTP/1.0";
        case t_http_version::HV_1_1:
            return "HTTP/1.1";
        default:
            return "UNKNOWN";
    }
}
