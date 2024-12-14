#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    const char* CPP_HOSTNAME = "httpstat.us";

    struct hostent* host_info = gethostbyname(CPP_HOSTNAME);
    if (host_info == NULL)
    {
        fprintf(stderr, "Error: Could not resolve hostname %s\n", CPP_HOSTNAME);
        return -1;
    }

    printf("Official name: %s\n", host_info->h_name);
    for (char** p_alias = host_info->h_aliases; *p_alias != NULL; p_alias++)
    {
        printf("Alias name %ld: %s\n", p_alias - host_info->h_aliases, *p_alias);
    }

    for (char** p_addr = host_info->h_addr_list; *p_addr != NULL; p_addr++)
    {
        printf("IP address %ld: %s\n", p_addr - host_info->h_addr_list, inet_ntoa(*(struct in_addr*)*p_addr));
    }

    struct addrinfo hints;
    struct addrinfo* res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(CPP_HOSTNAME, "80", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "Error: getaddrinfo failed");
        return -1;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    struct sockaddr_in server_address = *addr;
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_address.sin_addr.s_addr, ip_str, sizeof(ip_str));
    printf("IP address: %s\n", ip_str);
    printf("Port: %d\n", ntohs(server_address.sin_port));

    freeaddrinfo(res);

    return 0;
}