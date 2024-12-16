#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "http_connection_handler.h"

class HTTPServer
{
public:
    HTTPServer(int port);
    void start();

private:
    bool setup_socket(int port);

private:
    int sock_server;
};

#endif // HTTP_SERVER_H