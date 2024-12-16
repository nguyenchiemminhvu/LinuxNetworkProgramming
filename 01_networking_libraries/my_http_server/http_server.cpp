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

#include <poll.h>

HTTPServer::HTTPServer(int port)
    : sock_server(-1)
{
    if (this->setup_socket(port) == false)
    {
        LOGE("Server HTTP service is not ready");
    }
}

void HTTPServer::start()
{
    if (this->sock_server < 0)
    {
        return;
    }

    pollfd fds[MAX_CONNECTION];
    int nfds = 1;
    fds[0].fd = this->sock_server;
    fds[0].events = POLLIN;

    // Server Loop
    while (true)
    {
        LOGI("Server starts new poll()");
        int rc_poll = poll(fds, nfds, -1);
        if (rc_poll < 0)
        {
            LOGE("Server poll() failed");
            break;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == this->sock_server)
                {
                    sockaddr addr_client;
                    socklen_t addr_client_len = sizeof(sockaddr);
                    int sock_client = accept(this->sock_server, &addr_client, &addr_client_len);
                    if (sock_client < 0)
                    {
                        LOGE("Server accept() failed");
                    }
                    else
                    {
                        LOGI("A client is connected");
                        print_sockaddr_info(&addr_client);

                        if (nfds < MAX_CONNECTION)
                        {
                            fds[nfds].fd = sock_client;
                            fds[nfds].events = POLLIN;
                            nfds++;
                        }
                    }
                }
            }
            else
            {
                HTTPConnectionHandler handler;
                ClientActivity activity = handler.handle_client(fds[i].fd);
                if (activity != ClientActivity::WAITING)
                {
                    LOGI("A client is disconnected");
                    close(fds[i].fd);
                    fds[i].fd = fds[nfds - 1].fd;
                    fds[i].events = fds[nfds - 1].events;
                    nfds--;
                }
            }
        }
    }
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
        this->sock_server = -1;
        return false;
    }

    set_socket_nonblocking(this->sock_server);

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
        this->sock_server = -1;
        return false;
    }

    if (listen(this->sock_server, MAX_CONNECTION) != 0)
    {
        LOGE("Server listen() failed");
        freeaddrinfo(addr_server);
        close(this->sock_server);
        this->sock_server = -1;
        return false;
    }

    freeaddrinfo(addr_server);
    return true;
}
