#include <arpa/inet.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static const std::string g_ok_response = "HTTP/1.1 200 OK\r\n\r\n";
static const std::string g_not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
static const std::string g_server_error_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";

static const std::unordered_set<std::string> headers = { "test_example.html" };

constexpr int g_port = 4221;


struct RootHandler {
    static std::string Handle(const std::string& text) {
        if (text.empty()) {
            return g_ok_response;
        }
        if (headers.find(text) == headers.end()) {
            return g_not_found_response;
        }
        //TODO: impl headers searching
        return g_ok_response;
    }
};

struct EchoHandler {
    static std::string Handle(const std::string& text) {
        return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
            std::to_string(text.size()) + "\r\n\r\n" + text;
    }
};

static std::map<std::string, std::function<std::string(const std::string&)>> g_router_map =
{
    {"echo", EchoHandler::Handle},
    {"User-Agent", EchoHandler::Handle}
};

class RequestStatus
{
public:
    RequestStatus() = default;
    RequestStatus(const std::string& parsed_request)
    {
        std::istringstream ss(parsed_request);
        ss >> m_method;
        ss >> m_target;
        ss >> m_version;
    }
    std::string Execute()
    {
        if (m_method == "GET")
        {
            std::istringstream ss(m_target);
            std::vector<std::string> tokens;
            std::string token;
            while (std::getline(ss, token, '/'))
            {
                tokens.push_back(token);
            }
            switch (tokens.size()) {
                case 0:
                    return g_server_error_response;
                case 1:
                    return g_ok_response;
                case 2:
                    return RootHandler::Handle(tokens[1]);
                default:
                    const std::string& path = tokens[1];
                    auto route = g_router_map.find(path);
                    if (route != g_router_map.end())
                    {
                        return route->second(tokens[2]);
                    }
                    return g_not_found_response;
            }
        }
        return g_server_error_response;
    }
    friend std::ostream& operator<<(std::ostream& os, RequestStatus requestLine)
    {
        os << "Method: " << requestLine.m_method << std::endl;
        os << "Target: " << requestLine.m_target << std::endl;
        os << "Version: " << requestLine.m_version << std::endl;
        return os;
    }

private:
    std::string m_method;
    std::string m_target;
    std::string m_version;
};

class HttpResponse {
public:
    HttpResponse() = default;
    HttpResponse(const std::string& responseCode, const std::string& responseBody = "") : m_response_code(responseCode), m_response_body(responseBody)
    {}

    std::string str() const {
        return m_response_code + m_response_body;
    }
private:
    std::string m_response_code;
    std::string m_response_body;
};

class HttpRequest {
public:
    HttpRequest(const std::string& rawRequest)
    {
        if (!rawRequest.empty())
        {
            auto idx = rawRequest.find_first_of("\r\n");
            if (idx == std::string::npos) {
                throw std::runtime_error("Error while parsing HTTP method");
            }
            m_request_line = RequestStatus(rawRequest.substr(0, idx));
            m_headers = ParseHeaders(rawRequest.substr(idx + 2));
        }
    }

    std::string Execute() {
        for (const auto& router : g_router_map) {
            if (m_headers.find(router.first) != m_headers.end()) {
                std::cout << "Router: " << router.first << std::endl;
                std::cout << "Check: " << m_headers[router.first] << std::endl;
                return router.second(m_headers[router.first]);
            }
        }
        return m_request_line.Execute();
    }
    friend std::ostream& operator<<(std::ostream& os, const HttpRequest& request)
    {
        os << request.m_request_line;
        return os;
    }

private:
    //TODO: separate to diff function
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

std::string parse_request(const std::string& request) {
    HttpRequest req(request);
    return req.Execute();
}

std::mutex request_mutex;

// Функция для обработки клиента
void handle_client(int client_fd)
{
    std::string request;
    request.resize(1024);

    ssize_t bytes_read = recv(client_fd, request.data(), request.size() - 1, 0);
    if (bytes_read < 0)
    {
        std::cerr << "Failed to read from client\n";
        close(client_fd);
        return;
    }

    request[bytes_read] = '\0'; // Завершаем строку
    std::cout << "get request - " << request << std::endl;

    // Здесь должна быть ваша функция обработки запроса
    const std::string response =
        parse_request(request); // Предполагается, что эта функция определена
    std::cout << "response: " << response << std::endl;

    send(client_fd, response.data(), response.size(), 0);

    close(client_fd);
}

int main(int argc, char** argv)
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(g_port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Failed to bind to port " << g_port << std::endl;
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        std::cerr << "Listen failed\n";
        return 1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Server listening on port " << g_port << std::endl;

    while (true)
    {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr,
            (socklen_t*)&client_addr_len);
        if (client_fd < 0)
        {
            std::cerr << "Accept failed\n";
            break;
        }

        std::cout << "Client connected\n";
        std::thread client_thread(handle_client, client_fd);
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}
