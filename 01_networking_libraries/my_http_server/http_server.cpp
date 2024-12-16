#include "http_server.h"
#include "logging.h"
#include "defs.h"
#include "utils.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

HTTPServer::HTTPServer(int port)
{
    this->setup_socket(port);
}

void HTTPServer::start()
{

}

bool HTTPServer::setup_socket(int port)
{
    protoent* tcp_proto = getprotobyname(STR_TCP_PROTOCOL);
    if (tcp_proto == nullptr)
    {
        return false;
    }

    std::string s_port = std::to_string(port);
    
    addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = tcp_proto->p_proto;
    hints.ai_flags = AI_PASSIVE;
    addrinfo* addr_server;
    if (getaddrinfo(STR_LOCALHOST, s_port.c_str(), &hints, &addr_server) != 0)
    {
        LOGE("Server getaddrinfo() failed");
        return false;
    }

    this->sock_server = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (this->sock_server < 0)
    {
        LOGE("Server socket() failed");
        freeaddrinfo(addr_server);
        return false;
    }

    int rc_bind = 0;
    for (addrinfo* p = addr_server; p != nullptr; p = p->ai_next)
    {
        print_sockaddr_info(p->ai_addr);
        rc_bind = bind(this->sock_server, p->ai_addr, p->ai_addrlen);
        if (rc_bind == 0)
        {
            break;
        }
    }
    
    if (rc_bind != 0)
    {
        LOGE("Server bind() failed");
        freeaddrinfo(addr_server);
        close(this->sock_server);
        return false;
    }

    return true;
}
