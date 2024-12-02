#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>

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

    protoent* tcp_proto = getprotobyname(PROTOCOL);
    if (tcp_proto == NULL)
    {
        report_error("TCP protocol is not supported");
        return;
    }

    char server_port[6];
    memset(server_port, 0, 6);
    sprintf(server_port, "%d", htons(TCP_PORT));

    addrinfo addr_hints;
    memset(&addr_hints, 0, sizeof(addr_hints));
    addr_hints.ai_family = AF_INET;
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = tcp_proto->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(HOST_NAME, server_port, &addr_hints, &addr_server);
    if (rc != 0)
    {
        report_error("Can not resolve server hostname");
        return;
    }

    int sock_server = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (sock_server < 0)
    {
        report_error("Server socket() failed");
        freeaddrinfo(addr_server);
        return;
    }

    for (addrinfo* p_server = addr_server; p_server != NULL; p_server = p_server->ai_next)
    {
        rc = bind(sock_server, p_server->ai_addr, p_server->ai_addrlen);
        if (rc == 0)
        {
            break;
        }
    }

    if (rc != 0)
    {
        report_error("Server bind() failed");
        close(sock_server);
        freeaddrinfo(addr_server);
        return;
    }

    rc = listen(sock_server, 3);
    if (rc < 0)
    {
        report_error("Server listen() failed");
        close(sock_server);
        freeaddrinfo(addr_server);
        return;
    }

    // server loop
    int sock_client = -1;
    while (true)
    {
        printf("Server is ready to process new request\n");
        
        sockaddr_storage addr_client;
        socklen_t addr_len = sizeof(addr_client);
        sock_client = accept(sock_server, (sockaddr*)&addr_client, &addr_len);
        if (sock_client < 0)
        {
            printf("Server rejected a client connection\n");
            continue;
        }

        printf("Server accepted client connection %d\n", sock_client);

        char request_buffer[MESSAGE_SIZE];
        char response_buffer[MESSAGE_SIZE];
        while (true)
        {
            memset(request_buffer, 0, MESSAGE_SIZE);
            int received_bytes = recv(sock_client, request_buffer, MESSAGE_SIZE, 0);
            if (received_bytes <= 0)
            {
                report_error("Client is disconnected\n");
                break;
            }
            request_buffer[received_bytes] = '\0';

            printf("Received client request: %s\n", request_buffer);

            memset(response_buffer, 0, MESSAGE_SIZE);

            if (strcmp(request_buffer, "exit") == 0
            || strcmp(request_buffer, "quit") == 0
            || strcmp(request_buffer, "shutdown") == 0)
            {
                sprintf(response_buffer, "OK");
                rc = send(sock_client, response_buffer, MESSAGE_SIZE, 0);
                close(sock_client);
                break;
            }
            else if (strcmp(request_buffer, "time") == 0)
            {
                sprintf(response_buffer, "%d", time(NULL));
                rc = send(sock_client, response_buffer, MESSAGE_SIZE, 0);
            }
            else
            {
                sprintf(response_buffer, "Unknown request");
                rc = send(sock_client, response_buffer, MESSAGE_SIZE, 0);
            }

            if (rc <= 0)
            {
                report_error("Server send() failed");
            }
        }
    }

    close(sock_server);
    freeaddrinfo(addr_server);
    printf("Server exit with normal status\n");
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
        freeaddrinfo(addr_server);
        return;
    }

    for (addrinfo* p_server = addr_server; p_server != NULL; p_server = p_server->ai_next)
    {
        rc = connect(sock_client, p_server->ai_addr, p_server->ai_addrlen);
        if (rc == 0)
        {
            break;
        }
    }

    if (rc != 0)
    {
        report_error("client connect() failed");
        close(sock_client);
        freeaddrinfo(addr_server);
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

        // Remove newline character from the request buffer
        request_buffer[strcspn(request_buffer, "\n")] = 0;

        int request_buffer_len = strlen(request_buffer);
        int sent_bytes = send(sock_client, request_buffer, request_buffer_len, 0);
        if (sent_bytes <= 0)
        {
            report_error("Client sent request fail");
            continue;
        }

        char response_buffer[MESSAGE_SIZE];
        memset(response_buffer, 0, MESSAGE_SIZE);
        int received_bytes = recv(sock_client, response_buffer, MESSAGE_SIZE, 0);
        if (received_bytes <= 0)
        {
            report_error("Client recv() failed");
        }
        else
        {
            printf("Response: %s\n", response_buffer);
        }

        if (strcmp(request_buffer, "exit") == 0
         || strcmp(request_buffer, "quit") == 0
         || strcmp(request_buffer, "shutdown") == 0)
        {
            break;
        }
    }

    close(sock_client);
    freeaddrinfo(addr_server);
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