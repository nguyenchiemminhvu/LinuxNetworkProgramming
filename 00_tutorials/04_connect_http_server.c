#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const char* CPP_HOSTNAME = "httpstat.us";

int main()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if (sock_fd < 0)
    {
        fprintf(stderr, "Error: socket() return invalid fd\n");
        return -1;
    }

    struct addrinfo hints;
    struct addrinfo* res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(CPP_HOSTNAME, "80", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "Error: getaddrinfo() failed\n");
        return -1;
    }

    struct sockaddr server_addr = *(res->ai_addr);
    freeaddrinfo(res);

    rc = connect(sock_fd, &server_addr, sizeof(server_addr));
    if (rc != 0)
    {
        fprintf(stderr, "Error: connect() failed\n");
        return -1;
    }

    close(sock_fd);

    return 0;
}