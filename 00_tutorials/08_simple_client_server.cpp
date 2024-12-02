#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#define PROTOCOL "tcp"
#define TCP_PORT 45123
#define MESSAGE_SIZE 1024
#define HOST_NAME "localhost"

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <client|server>\n", program_name);
}

void report_error(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
}

void run_server()
{
    int rc;

    // server loop
    while (true)
    {
        printf("Server is ready to process new request\n");
        sleep(1);
    }
}

void run_client()
{
    int rc;

    protoent* tcp_proto = getprotobyname(PROTOCOL);
    if (tcp_proto == NULL)
    {
        report_error("TCP protocol is not supported");
        return;
    }

    char server_port[6];
    memset(server_port, 0, 6);
    sprintf(server_port, "%d", ntohs(TCP_PORT));

    addrinfo addr_hints;
    memset(&addr_hints, 0, sizeof(addr_hints));
    addr_hints.ai_family = AF_INET;
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = tcp_proto->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(HOST_NAME, server_port, &addr_hints, &addr_server);
    if (rc != 0)
    {
        report_error("Failed to resolve hostname");
        return;
    }

    int sock_client = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (sock_client < 0)
    {
        report_error("client socket() failed");
        return;
    }

    rc = connect(sock_client, addr_server->ai_addr, sizeof(sockaddr));
    if (rc != 0)
    {
        report_error("client connect() failed");
        return;
    }

    // client loop
    while (true)
    {
        printf("Client is ready to create new request\n");
        char request_buffer[MESSAGE_SIZE];
        memset(request_buffer, 0, MESSAGE_SIZE);

        printf("Enter command: ");
        fgets(request_buffer, MESSAGE_SIZE, stdin);
        printf("Request: %s\n", request_buffer);

        int request_buffer_len = strlen(request_buffer);
        int total_sent_bytes = 0;
        while (total_sent_bytes < request_buffer_len)
        {
            int sent_bytes = send(sock_client, request_buffer + total_sent_bytes, request_buffer_len - total_sent_bytes, 0);
            if (sent_bytes <= 0)
            {
                break;
            }

            total_sent_bytes += sent_bytes;
        }

        if (total_sent_bytes < request_buffer_len)
        {
            report_error("Client sent request fail");
            continue;
        }

        char response_buffer[MESSAGE_SIZE];
        memset(response_buffer, 0, MESSAGE_SIZE);
        int total_received_bytes = 0;
        int received_bytes = recv(sock_client, response_buffer + total_received_bytes, MESSAGE_SIZE - total_received_bytes, 0);
        while (received_bytes > 0)
        {
            total_received_bytes += received_bytes;
            received_bytes = recv(sock_client, response_buffer + total_received_bytes, MESSAGE_SIZE - total_received_bytes, 0);
        }

        printf("Response: %s\n", response_buffer);

        if (strcmp(request_buffer, "exit") == 0
         || strcmp(request_buffer, "quit") == 0
         || strcmp(request_buffer, "shutdown") == 0)
        {
            break;
        }

        sleep(1);
    }

    printf("Client exit with normal status\n");
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "server") == 0)
    {
        run_server();
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        run_client();
    }
    else
    {
        print_usage(argv[0]);
        return -1;
    }

    return 0;
}