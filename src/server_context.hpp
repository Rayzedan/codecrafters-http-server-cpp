#ifndef SERVER_CONTEXT_HPP
#define SERVER_CONTEXT_HPP

#include <string>
#include "common.hpp"
#include <map>

class ServerContext
{
public:
    ServerContext() = default;
    ServerContext(const std::map<t_server_ctx, std::string>& args) : m_ctx(args)
    {
    }
    std::string GetDirectory() const
    {
        auto it = m_ctx.find(t_server_ctx::SC_DIRECTORY);
        if (it != m_ctx.end())
        {
            return it->second;
        }
        return {};
    }
private:
    std::map<t_server_ctx, std::string> m_ctx;
};

#endif // !SERVER_CONTEXT_HPP
