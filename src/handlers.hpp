#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <string>
#include "http_request.hpp"
#include "server_context.hpp"

std::string handle_http_request(const HttpRequest& request, const ServerContext& ctx);

#endif
