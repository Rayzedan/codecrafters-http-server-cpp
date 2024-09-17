#include "common.hpp"
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>

std::vector<std::string> split(const std::string& s)
{
    std::vector<std::string> tokens;
    std::istringstream ss(s);
    std::string token;

    if (s == "/")
    {
        tokens.push_back("/");
        return tokens;
    }
    while (std::getline(ss, token, '/'))
    {
        if (!token.empty())
        {
            tokens.push_back("/");
            tokens.push_back(std::move(token));
        }
    }
    if (s.back() == '/')
    {
        tokens.push_back("/");
    }
    return tokens;
}

std::string read_file(const std::string& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        return {};
    }
    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    return content;
}

bool write_file(const std::string& path, const std::string& content, size_t size)
{
    std::ofstream file(path, std::ios::out);
    if (!file.is_open())
    {
        return false;
    }
    file.write(content.c_str(), size);
    return true;
}

std::map<t_server_ctx, std::string> parse_args(int argc, char** argv)
{
    const option options[] =
    {
        {"directory", required_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };
    std::map<t_server_ctx, std::string> args;
    for (;;)
    {
        int i = 0;
        int v = getopt_long(argc, argv, "d", options, &i);
        if (v == -1)
        {
            break;
        }
        switch (v)
        {
            case 'd':
                args[t_server_ctx::SC_DIRECTORY] = optarg;
                break;
            default:
                abort();
        }
    }
    return args;
}
