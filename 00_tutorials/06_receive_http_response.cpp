#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

const char* CPP_HOSTNAME = "cppinstitute.org";
const int MESSAGE_SIZE = 1024;

void on_func_failure(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

int main()
{
    protoent* p_proto_ent = getprotobyname("tcp");
    if (p_proto_ent == NULL)
    {
        on_func_failure("TCP protocol is not available");
    }

    servent* p_service_ent = getservbyname("http", p_proto_ent->p_name);
    if (p_service_ent == NULL)
    {
        on_func_failure("HTTP service is not available");
    }

    char port_buffer[6];
    memset(port_buffer, 0, sizeof(port_buffer));
    sprintf(port_buffer, "%d", ntohs(p_service_ent->s_port));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = p_proto_ent->p_proto;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* server_addr;
    int rc = getaddrinfo(CPP_HOSTNAME, port_buffer, &hints, &server_addr);
    if (rc != 0)
    {
        on_func_failure("Failed to resolve hostname");
    }

    int sock_fd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol);
    if (sock_fd < 0)
    {
        freeaddrinfo(server_addr);
        on_func_failure("socket() failed");
    }

    rc = connect(sock_fd, server_addr->ai_addr, sizeof(sockaddr));
    if (rc != 0)
    {
        freeaddrinfo(server_addr);
        on_func_failure("connect() failed");
    }

    char http_request[MESSAGE_SIZE];
    memset(http_request, 0, MESSAGE_SIZE);
    sprintf(http_request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", CPP_HOSTNAME);
    
    int http_request_len = strlen(http_request);
    int sent_bytes = 0;
    while (sent_bytes < http_request_len)
    {
        int sent_rc = send(sock_fd, http_request + sent_bytes, http_request_len - sent_bytes, 0);
        if (sent_rc < 0)
        {
            close(sock_fd);
            freeaddrinfo(server_addr);
            on_func_failure("send() failed");
        }
        printf("sent %d bytes\n", sent_rc);
        sent_bytes += sent_rc;
    }

    char http_response[MESSAGE_SIZE];
    memset(http_response, 0, MESSAGE_SIZE);
    int received_bytes = 0;
    while (1 == 1)
    {
        int received_rc = recv(sock_fd, http_response + received_bytes, MESSAGE_SIZE - received_bytes, 0);
        if (received_rc <= 0)
        {
            break;
        }

        printf("Received %d bytes\n", received_rc);
        received_bytes += received_rc;
    }

    if (received_bytes <= 0)
    {
        close(sock_fd);
        freeaddrinfo(server_addr);
        on_func_failure("recv() failed");
    }

    printf("HTTP Response:\n%s\n", http_response);

    close(sock_fd);
    freeaddrinfo(server_addr);

    return 0;
}