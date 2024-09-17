#ifndef COMMON_HPP
#define COMMON_HPP

#include <vector>
#include <map>
#include <string>
#include <zlib.h>

enum class t_server_ctx
{
    SC_DIRECTORY = 0,
    SC_UNKNOWN
};

std::vector<std::string> split(const std::string& s);
std::string read_file(const std::string& path);
std::map<t_server_ctx, std::string> parse_args(int argc, char** argv);
bool write_file(const std::string& path, const std::string& content, size_t size);

std::string compress(const std::string& content);

#endif
