#include "utils.h"
#include "logging.h"
#include <cstring>
#include <fcntl.h>

void print_sockaddr_info(sockaddr* sa)
{
    char ip[INET6_ADDRSTRLEN];
    std::memset(ip, 0, INET6_ADDRSTRLEN);

    void *addr;
    int port;

    if (sa->sa_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)sa;
        addr = &(sin->sin_addr);
        port = ntohs(sin->sin_port);
    }
    else if (sa->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
        addr = &(sin6->sin6_addr);
        port = ntohs(sin6->sin6_port);
    }
    else
    {
        LOGE("Unknown address family");
        return;
    }

    if (inet_ntop(sa->sa_family, addr, ip, sizeof(ip)) == NULL)
    {
        LOGE("inet_ntop() failed");
        return;
    }

    std::string ip_port = std::string(ip) + ":" + std::to_string(port);
    LOGI(ip_port);
}

void set_socket_nonblocking(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1)
    {
        LOGE("fcntl(F_GETFL) failed");
        return;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        LOGE("fcntl(F_SETFL, O_NONBLOCK) failed");
    }
}