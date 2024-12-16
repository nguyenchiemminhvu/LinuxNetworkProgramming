#ifndef HTTP_CONNECTION_HANDLER_H
#define HTTP_CONNECTION_HANDLER_H

#include "http_parser.h"
#include "http_router.h"

class HTTPConnectionHandler
{
public:
    HTTPConnectionHandler(int sock_client);
};

#endif // HTTP_CONNECTION_HANDLER_H