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
