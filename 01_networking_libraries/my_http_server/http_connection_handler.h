#ifndef HTTP_CONNECTION_HANDLER_H
#define HTTP_CONNECTION_HANDLER_H

#include "defs.h"
#include "http_parser.h"
#include "http_router.h"

class HTTPConnectionHandler
{
public:
    HTTPConnectionHandler();
    ClientActivity handle_client(int sock_client);

private:
    HTTPParser m_parser;
    HTTPRouter m_router;
};

#endif // HTTP_CONNECTION_HANDLER_H