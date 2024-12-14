#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

const char* CPP_HOSTNAME = "httpstat.us";

int main()
{
    struct protoent* p_proto = getprotobyname("tcp");
    if (p_proto == NULL)
    {
        fprintf(stderr, "Error: TCP protocol is not available\n");
        return -1;
    }

    struct servent* p_service = getservbyname("http", p_proto->p_name);
    if (p_service == NULL)
    {
        fprintf(stderr, "Error: HTTP service is not available\n");
        return -1;
    }
    char port_str[6] = {'\0', };
    sprintf(port_str, "%d", ntohs(p_service->s_port));

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = p_proto->p_proto;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res;
    int rc = getaddrinfo(CPP_HOSTNAME, port_str, &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "Error: Can not resolve hostname %s\n", CPP_HOSTNAME);
        return -1;
    }

    struct sockaddr server_addr = *(res->ai_addr);

    int sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd < 0)
    {
        fprintf(stderr, "Error: socket() failed\n");
        return -1;
    }

    rc = connect(sock_fd, &server_addr, sizeof(server_addr));
    if (rc != 0)
    {
        fprintf(stderr, "Error: connect() failed\n");
        return -1;
    }

    char request[1024] = {'\0', };
    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", CPP_HOSTNAME);
    ssize_t request_len = strlen(request);
    ssize_t sent_bytes = 0;
    while (sent_bytes < request_len)
    {
        ssize_t sent = send(sock_fd, request, strlen(request), 0);
        if (rc < 0)
        {
            fprintf(stderr, "Error: send() failed\n");
            close(sock_fd);
            return -1;
        }

        sent_bytes += sent;
        printf("sent %d bytes\n", sent_bytes);
    }

    close(sock_fd);
    freeaddrinfo(res);

    return 0;
}