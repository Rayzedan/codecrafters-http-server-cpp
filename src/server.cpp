#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct EchoHandler {
    static std::string Handle(const std::string& text) {
        std::cout << "Here" << std::endl;
        return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
            std::to_string(text.size()) + "\r\n\r\n" + text;
    }
};

static const std::unordered_set<std::string> headers = { "test_example.html" };

static std::map<std::string, std::function<std::string(const std::string&)>> g_handler_map = {
    {"echo", EchoHandler::Handle} // Указатель на статический метод
};

static const std::string ok_response = "HTTP/1.1 200 OK\r\n\r\n";
static const std::string not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";


std::string handle_endpoints(const std::string& endpoint)
{
    std::istringstream ss(endpoint);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(ss, token, '/'))
    {
        tokens.push_back(token);
    }
    switch (tokens.size()) {
        case 0:
            return not_found_response;
        case 1: {
            return tokens[0].empty() ? ok_response : not_found_response;
        }
        case 2: {
            if (headers.find(tokens[1]) != headers.end()) {
                return ok_response;
            }
            return not_found_response;
        }
        default: {
            auto handler = g_handler_map.find(tokens[1]);
            if (handler != g_handler_map.end()) {
                return handler->second(tokens[2]);
            }
        }
    }
    return not_found_response;
}

std::string handle_get_request(const std::string& method, const std::string& target) {
    if (method == "GET") {
        return handle_endpoints(target);
    }
    return "HTTP/1.1 200 OK\r\n\r\n";
}

std::string handle_request(const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2)
    {
        return "HTTP/1.1 404 Not Found\r\n\r\n";
    }
    return handle_get_request(tokens[0], tokens[1]);
}

std::string parse_request(const std::string& request) {
    auto idx = request.find_first_of("\r\n");
    if (idx != std::string::npos) {
        std::istringstream ss(request.substr(0, idx));
        std::vector<std::string> tokens;
        std::string token;
        while (ss >> token) {
            tokens.push_back(token);
        }
        return handle_request(tokens);
    }
    return {};
}

int main(int argc, char** argv)
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";

    // Uncomment this block to pass the first stage

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    // // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        std::cerr << "listen failed\n";
        return 1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
    if (client_fd < 0)
    {
        std::cerr << "accept failed\n";
        return 1;
    }
    std::cout << "Client connected\n";
    std::string request;
    request.resize(1024);
    ssize_t bytes_read = recv(client_fd, request.data(), request.size() - 1, 0);
    parse_request(request);
    const std::string response = parse_request(request);
    std::cout << "response - " << response << std::endl;
    send(client_fd, response.data(), response.size(), 0);
    close(server_fd);
    return 0;
}
