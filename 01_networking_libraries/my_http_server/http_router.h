#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include "http_request.h"
#include "http_response.h"

class HTTPRouter
{
public:
    HTTPRouter() = default;
    HTTPResponse route(const HTTPRequest& request);
};

#endif // HTTP_ROUTER_H