#include "utils.h"
#include "logging.h"
#include <cstring>

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