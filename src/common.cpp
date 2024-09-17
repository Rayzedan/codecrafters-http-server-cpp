#include "common.hpp"
#include <cstring>
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

std::string compress(const std::string& content)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8,
            Z_DEFAULT_STRATEGY) != Z_OK)
    {
        throw std::runtime_error("deflateInit2 failed while compressing.");
    }
    zs.next_in = (Bytef*)content.data();
    zs.avail_in = content.size();
    int ret;
    char outbuffer[32768];
    std::string out;
    do
    {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        ret = deflate(&zs, Z_FINISH);
        if (out.size() < zs.total_out)
        {
            out.append(outbuffer, zs.total_out - out.size());
        }

    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
    {
        throw std::runtime_error("Exception during zlib compression: (" + std::to_string(ret) + ") " + zs.msg);
    }

    return out;
}
