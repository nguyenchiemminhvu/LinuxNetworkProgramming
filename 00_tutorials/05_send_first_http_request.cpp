#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

const char* CPP_HOSTNAME = "cppinstitute.org";

int main()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if (sock_fd < 0)
    {
        fprintf(stderr, "Error: socket() failed\n");
        return -1;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* res;
    int rc = getaddrinfo(CPP_HOSTNAME, "80", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "Error: resolve hostname failed\n");
        return -1;
    }

    sockaddr server_addr = *(res->ai_addr);
    freeaddrinfo(res);

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
            return -1;
        }

        sent_bytes += sent;
        printf("sent %d bytes\n", sent_bytes);
    }

    close(sock_fd);

    return 0;
}