#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <pthread.h>

#define PROTOCOL "tcp"
#define TCP_PORT 45123
#define MESSAGE_SIZE 1024
#define HOST_NAME "localhost"
#define MAX_CONNECTION 100

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <client|server>\n", program_name);
}

void report_error(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
}

void* server_handle_client(void* arg)
{
    int* sock_client = ((int*)arg);
    if (sock_client == NULL)
    {
        return NULL;
    }

    int rc;
    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];
    while (true)
    {
        memset(request_buffer, 0, MESSAGE_SIZE);
        memset(response_buffer, 0, MESSAGE_SIZE);

        int received_bytes = recv(*sock_client, request_buffer, MESSAGE_SIZE, 0);
        if (received_bytes <= 0)
        {
            report_error("Client is disconnected");
            break;
        }
        request_buffer[received_bytes] = '\0';

        printf("Client %d requests: %s\n", *sock_client, request_buffer);

        if (strcmp(request_buffer, "exit") == 0
        || strcmp(request_buffer, "quit") == 0
        || strcmp(request_buffer, "shutdown") == 0)
        {
            sprintf(response_buffer, "OK");
            rc = send(*sock_client, response_buffer, MESSAGE_SIZE, 0);
            close(*sock_client);
            break;
        }
        else if (strcmp(request_buffer, "time") == 0)
        {
            sprintf(response_buffer, "%d", time(NULL));
            rc = send(*sock_client, response_buffer, MESSAGE_SIZE, 0);
        }
        else
        {
            sprintf(response_buffer, "Unknown request");
            rc = send(*sock_client, response_buffer, MESSAGE_SIZE, 0);
        }

        if (rc <= 0)
        {
            report_error("Server send() failed");
        }
    }

    if (sock_client != NULL)
    {
        free(sock_client);
    }

    return NULL;
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

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = tcp_proto->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(NULL, server_port, &hints, &addr_server);
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

    int sock_server_opt = 1;
    rc = setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE, &sock_server_opt, sizeof(sock_server_opt));
    if (rc < 0)
    {
        report_error("Server setsockopt() failed");
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
        freeaddrinfo(addr_server);
        close(sock_server);
        return;
    }

    rc = listen(sock_server, MAX_CONNECTION);
    if (rc < 0)
    {
        report_error("Server listen() failed");
        freeaddrinfo(addr_server);
        close(sock_server);
        return;
    }

    // server loop
    while (true)
    {
        printf("Server is ready to process new request\n");

        sockaddr addr_client;
        socklen_t addr_client_len = sizeof(addr_client);
        int sock_client = accept(sock_server, &addr_client, &addr_client_len);
        if (sock_client < 0)
        {
            report_error("Server accept() failed");
            continue;
        }

        char ip_client[NI_MAXHOST];
        char service_client[NI_MAXSERV];
        rc = getnameinfo(&addr_client, addr_client_len, ip_client, sizeof(ip_client), service_client, sizeof(service_client), NI_NUMERICHOST | NI_NUMERICSERV);
        if (rc == 0)
        {
            printf("Server accepted client connection %s:%s\n", ip_client, service_client);
        }

        int* p_sock_client = (int*)calloc(1, sizeof(int));
        *p_sock_client = sock_client;
        pthread_t client_thread;
        rc = pthread_create(&client_thread, NULL, server_handle_client, p_sock_client);
        if (rc != 0)
        {
            report_error("Server create thread for new client failed");
            continue;
        }

        rc = pthread_detach(client_thread);
        if (rc != 0)
        {
            report_error("Detach client thread failed");
        }
    }

    freeaddrinfo(addr_server);
    close(sock_server);
}

void run_client()
{
    int rc;

    protoent* tcp_proto = getprotobyname(PROTOCOL);
    if (tcp_proto == NULL)
    {
        report_error("TCP protocol is not available");
        return;
    }

    char server_port[6];
    memset(server_port, 0, 6);
    sprintf(server_port, "%d", htons(TCP_PORT));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = tcp_proto->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(HOST_NAME, server_port, &hints, &addr_server);
    if (rc != 0)
    {
        report_error("Can not resolve hostname");
        return;
    }

    int sock_client = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (sock_client < 0)
    {
        report_error("Client socket() failed");
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
        report_error("Client connect() failed");
        freeaddrinfo(addr_server);
        close(sock_client);
        return;
    }

    // client loop
    while (true)
    {
        printf("Client is ready to reate a new request\n");
        char request_buffer[MESSAGE_SIZE];
        memset(request_buffer, 0, MESSAGE_SIZE);

        printf("Enter command: ");
        fgets(request_buffer, MESSAGE_SIZE, stdin);
        request_buffer[strcspn(request_buffer, "\r\n")] = 0;

        printf("Request: %s\n", request_buffer);

        int request_buffer_len = strlen(request_buffer);
        int sent_bytes = send(sock_client, request_buffer, request_buffer_len, 0);
        if (sent_bytes != request_buffer_len)
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

    freeaddrinfo(addr_server);
    close(sock_client);

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