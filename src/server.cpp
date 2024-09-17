#include "http_request.hpp"
#include "handlers.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr uint16_t g_port = 4221;

std::string parse_request(const std::string& request, const ServerContext& ctx)
{
    HttpRequest req(request);
    std::cout << "REQUEST: " << req << std::endl;
    return handle_http_request(req, ctx).str();
}

std::mutex request_mutex;

void handle_client(int client_fd, const ServerContext& ctx)
{
    std::string request;
    request.resize(1024);
    const ssize_t bytes_read = recv(client_fd, request.data(), request.size() - 1, 0);
    if (bytes_read < 0)
    {
        std::cerr << "Failed to read from client\n";
        close(client_fd);
        return;
    }
    request[bytes_read] = '\0';
    const std::string response =
        parse_request(request, ctx);
    send(client_fd, response.data(), response.size(), 0);
    close(client_fd);
}

int main(int argc, char** argv)
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    ServerContext ctx(parse_args(argc, argv));
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "Setsockopt failed\n";
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

        std::cout << "Client connected " << inet_ntoa(client_addr.sin_addr) << std::endl;
        std::thread client_thread(handle_client, client_fd, ctx);
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}
