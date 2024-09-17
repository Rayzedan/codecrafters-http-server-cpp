#include "http_response.hpp"

std::string to_string(t_response_answer type)
{
    switch (type)
    {
        case t_response_answer::RT_OK:
            return "200 OK";
        case t_response_answer::RT_CREATED:
            return "201 Created";
        case t_response_answer::RT_NOT_FOUND:
            return "404 Not Found";
        case t_response_answer::RT_SERVER_ERROR:
            return "500 Internal Server Error";
        default:
            return "UNKNOWN";
    }
}
