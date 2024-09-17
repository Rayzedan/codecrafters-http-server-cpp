#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "http_request.hpp"
#include "http_response.hpp"
#include "server_context.hpp"

HttpResponse handle_http_request(const HttpRequest& request, const ServerContext& ctx);

#endif
