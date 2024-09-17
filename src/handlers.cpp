#include "handlers.hpp"
#include "common.hpp"
#include <functional>
#include <unordered_set>

static const std::string g_ok_response = "HTTP/1.1 200 OK\r\n\r\n";
static const std::string g_not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
static const std::string g_server_error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
static const std::unordered_set<std::string> g_headers = {"test_example.html"};

struct RootHandler
{
    static HttpResponse Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const auto& status = request.GetStatus();
        if (status.GetMethod() == t_request_type::RT_GET)
        {
            return HttpResponse(t_response_answer::RT_OK, t_http_version::HV_1_1);
        }
        return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
    }
};

struct FileHandler
{
    static HttpResponse Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const std::string& path = ctx.GetDirectory();
        const auto& status = request.GetStatus();
        const auto& target = status.GetTarget();
        if (path.empty() || target.size() < 4)
        {
            return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
        }
        switch (status.GetMethod())
        {
            case t_request_type::RT_POST:
                return handle_post(path + '/' + target[3], request);
            case t_request_type::RT_GET:
                return handle_get(path + '/' + target[3], request);
            default:
                return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
        }
        return HttpResponse(t_response_answer::RT_SERVER_ERROR, t_http_version::HV_1_1);
    }
private:
    static HttpResponse handle_post(const std::string& path, const HttpRequest& request)
    {
        const auto& headers = request.GetHeaders();
        auto it = headers.find("Content-Length");
        if (it == headers.end())
        {
            return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
        }
        const size_t size = std::stoi(it->second);
        if (!write_file(path, request.GetBody(), size))
        {
            return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
        }
        return HttpResponse(t_response_answer::RT_CREATED, t_http_version::HV_1_1);
    }
    static HttpResponse handle_get(const std::string& path, const HttpRequest& request)
    {
        const auto& headers = request.GetHeaders();
        const std::string file = read_file(path);
        if (file.empty())
        {
            return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
        }
        HttpResponse response(t_response_answer::RT_OK, t_http_version::HV_1_1);
        response.SetContentType("application/octet-stream");
        response.SetContentLength(file.size());
        auto encoding = headers.find("Accept-Encoding");
        //TODO: add encoding to server ctx
        if (encoding != headers.end() && encoding->second == "gzip")
        {
            response.SetEncoding("gzip");
        }
        response.SetBody(file);
        return response;
    }
};

struct EchoHandler
{
    static HttpResponse Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const auto& status = request.GetStatus();
        if (status.GetMethod() == t_request_type::RT_GET)
        {
            const auto& targets = status.GetTarget();
            if (targets.empty() || targets.size() < 4)
            {
                return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
            }
            HttpResponse response(t_response_answer::RT_OK, t_http_version::HV_1_1);
            response.SetContentType("text/plain");
            const auto& headers = request.GetHeaders();
            auto encoding = headers.find("Accept-Encoding");
            if (encoding != headers.end() && encoding->second == "gzip")
            {
                response.SetEncoding("gzip");
            }
            response.SetContentLength(targets[3].size());
            response.SetBody(targets[3]);
            return response;
        }
        return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
    }
};

struct UserAgentHandler
{
    static HttpResponse Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const auto& status = request.GetStatus();
        if (status.GetMethod() == t_request_type::RT_GET)
        {
            const auto& headers = request.GetHeaders();
            const auto& userAgent = headers.find("User-Agent");
            if (userAgent == headers.end())
            {
                return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
            }
            HttpResponse response(t_response_answer::RT_OK, t_http_version::HV_1_1);
            response.SetContentType("text/plain");
            response.SetContentLength(userAgent->second.size());
            auto encoding = headers.find("Accept-Encoding");
            if (encoding != headers.end() && encoding->second == "gzip")
            {
                response.SetEncoding("gzip");
            }
            response.SetBody(userAgent->second);
            return response;
        }
        return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
    }
};

struct HtmlHandler
{
    static HttpResponse Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const std::string& path = ctx.GetDirectory();
        if (path.empty())
        {
            return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
        }
        const auto& status = request.GetStatus();
        const auto& target = status.GetTarget();
        std::string data = read_file(path + '/' + target[1]);

        if (data.empty())
        {
            return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
        }

        HttpResponse response(t_response_answer::RT_OK, t_http_version::HV_1_1);
        response.SetContentType("text/html");
        response.SetContentLength(data.size());
        const auto& headers = request.GetHeaders();
        auto encoding = headers.find("Accept-Encoding");
        if (encoding != headers.end() && encoding->second == "gzip")
        {
            response.SetEncoding("gzip");
        }
        response.SetBody(data);
        return response;
    }
};

static std::map<std::string, std::function<HttpResponse(const HttpRequest&, const ServerContext&)> >g_router_map =
    {
        {"/", RootHandler::Handle},
        {"/echo", EchoHandler::Handle},
        {"/files", FileHandler::Handle},
        {"/user-agent", UserAgentHandler::Handle},
        {"/test_example.html", HtmlHandler::Handle}
    };

HttpResponse handle_http_request(const HttpRequest& request, const ServerContext& ctx)
{
    const auto& status = request.GetStatus();
    const auto& target = status.GetTarget();
    if (target.empty())
    {
        return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
    }
    const auto& headers = request.GetHeaders();
    const std::string key = target[0] + (target.size() > 1 ? target[1] : "");
    const auto& route = g_router_map.find(key);
    if (route != g_router_map.end())
    {
        return route->second(request, ctx);
    }
    return HttpResponse(t_response_answer::RT_NOT_FOUND, t_http_version::HV_1_1);
}
