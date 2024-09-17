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
    static std::string Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const auto& status = request.GetStatus();
        if (status.GetMethod() == t_request_type::RT_GET)
        {
            return g_ok_response;
        }
        return g_server_error_response;
    }
};

struct FileHandler
{
    static std::string Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const std::string& path = ctx.GetDirectory();
        const auto& status = request.GetStatus();
        const auto& target = status.GetTarget();
        if (path.empty() || target.size() < 4)
        {
            return g_not_found_response;
        }
        switch (status.GetMethod())
        {
            case t_request_type::RT_POST:
                return handle_post(path + '/' + target[3], request);
            case t_request_type::RT_GET:
                return handle_get(path + '/' + target[3]);
            default:
                return g_server_error_response;
        }
        return g_server_error_response;
    }
private:
    static std::string handle_post(const std::string& path, const HttpRequest& request)
    {
        const auto& headers = request.GetHeaders();
        auto it = headers.find("Content-Length");
        if (it == headers.end())
        {
            return g_not_found_response;
        }
        const size_t size = std::stoi(it->second);
        std::cout << "Size: " << size << std::endl;
        std::cout << "Body: " << request.GetBody() << std::endl;
        if (!write_file(path, request.GetBody(), size))
        {
            return g_not_found_response;
        }
        return "HTTP/1.1 201 Created\r\n\r\n";
    }
    static std::string handle_get(const std::string& path)
    {
        const std::string file = read_file(path);
        if (file.empty())
        {
            return g_not_found_response;
        }
        return "HTTP/1.1 200 OK\r\nContent-Type: "
            "application/octet-stream\r\nContent-Length: " +
            std::to_string(file.size()) + "\r\n\r\n" + file;
    }
};

struct EchoHandler
{
    static std::string Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const auto& status = request.GetStatus();
        if (status.GetMethod() == t_request_type::RT_GET)
        {
            const auto& targets = status.GetTarget();
            if (targets.empty() || targets.size() < 4)
            {
                return g_not_found_response;
            }
            for (const auto& target : targets)
            {
                std::cout << "target: " << target << std::endl;
            }
            return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
                std::to_string(targets[3].size()) + "\r\n\r\n" + targets[3];
        }
        return g_server_error_response;
    }
};

struct UserAgentHandler
{
    static std::string Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const auto& status = request.GetStatus();
        if (status.GetMethod() == t_request_type::RT_GET)
        {
            const auto& headers = request.GetHeaders();
            const auto& userAgent = headers.find("User-Agent");
            if (userAgent == headers.end())
            {
                return g_not_found_response;
            }
            return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
                std::to_string(userAgent->second.size()) + "\r\n\r\n" +
                userAgent->second;
        }
        return g_server_error_response;
    }
};

struct HtmlHandler
{
    static std::string Handle(const HttpRequest& request, const ServerContext& ctx)
    {
        const std::string& path = ctx.GetDirectory();
        if (path.empty())
        {
            return g_not_found_response;
        }
        const auto& status = request.GetStatus();
        const auto& target = status.GetTarget();
        std::string response = read_file(path + '/' + target[1]);

        if (response.empty())
        {
            return g_not_found_response;
        }
        return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " +
            std::to_string(response.size()) + "\r\n\r\n" + response;
    }
};

static std::map<std::string, std::function<std::string(const HttpRequest&, const ServerContext&)> >g_router_map =
    {
        {"/", RootHandler::Handle},
        {"/echo", EchoHandler::Handle},
        {"/files", FileHandler::Handle},
        {"/user-agent", UserAgentHandler::Handle},
        {"/test_example.html", HtmlHandler::Handle}
    };

std::string handle_http_request(const HttpRequest& request, const ServerContext& ctx)
{
    const auto& status = request.GetStatus();
    const auto& target = status.GetTarget();
    if (target.empty())
    {
        return g_not_found_response;
    }
    const auto& headers = request.GetHeaders();
    const std::string key = target[0] + (target.size() > 1 ? target[1] : "");
    const auto& route = g_router_map.find(key);
    if (route != g_router_map.end())
    {
        return route->second(request, ctx);
    }
    return g_not_found_response;
}
