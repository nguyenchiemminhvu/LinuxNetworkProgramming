#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <errno.h>

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

void set_non_blocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1)
    {
        report_error("fcntl(F_GETFL) failed");
        return;
    }

    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        report_error("fcntl(F_SETFL) failed");
    }
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

    char port_server[6];
    memset(port_server, 0, 6);
    sprintf(port_server, "%d", htons(TCP_PORT));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = tcp_proto->p_proto;
    hints.ai_flags = AI_PASSIVE;
    addrinfo* addr_server;
    rc = getaddrinfo(HOST_NAME, port_server, &hints, &addr_server);
    if (rc != 0)
    {
        report_error("Server getaddrinfo() failed");
        return;
    }

    int sock_server = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (sock_server < 0)
    {
        report_error("Server socket() failed");
        freeaddrinfo(addr_server);
        return;
    }

    set_non_blocking(sock_server);

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
    if (rc != 0)
    {
        report_error("Server listen() failed");
        freeaddrinfo(addr_server);
        close(sock_server);
        return;
    }

    // Server Loop
    int sock_client = -1;
    char buffer[MESSAGE_SIZE];
    while (true)
    {
        sockaddr addr_client;
        socklen_t addr_client_len = sizeof(sockaddr);
        if (sock_client < 0)
        {
            sock_client = accept(sock_server, &addr_client, &addr_client_len);
            if (sock_client < 0)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    report_error("Server accept() failed");
                    break;
                }
                else
                {
                    printf("No client connection\n");
                }
            }
            else
            {
                char ip_client[NI_MAXHOST];
                char service_client[NI_MAXSERV];
                rc = getnameinfo(&addr_client, addr_client_len, ip_client, sizeof(ip_client), service_client, sizeof(service_client), NI_NUMERICHOST | NI_NUMERICSERV);
                if (rc == 0)
                {
                    printf("Server accepted client connection %s:%s\n", ip_client, service_client);
                }

                set_non_blocking(sock_client);
            }
        }

        if (sock_client > 0)
        {
            memset(buffer, 0, MESSAGE_SIZE);
            int received_bytes = recv(sock_client, buffer, MESSAGE_SIZE, 0);
            if (received_bytes < 0)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    report_error("Server recv() failed");
                    break;
                }
            }
            else if (received_bytes == 0)
            {
                printf("Client is disconnected\n");
            }
            else
            {
                buffer[received_bytes] = 0;
                printf("Server received data: %s\n", buffer);
                int sent_bytes = send(sock_client, "Status OK", 10, 0);
                if (sent_bytes != 10)
                {
                    report_error("Server send() failed");
                }
            }
        }
    }

    close(sock_server);
    close(sock_client);
    freeaddrinfo(addr_server);
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

    char port_server[6];
    memset(port_server, 0, 6);
    sprintf(port_server, "%d", htons(TCP_PORT));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = tcp_proto->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(HOST_NAME, port_server, &hints, &addr_server);
    if (rc != 0)
    {
        report_error("Server getaddrinfo() failed");
        return;
    }

    int sock_client = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (sock_client < 0)
    {
        report_error("Client socket() failed");
        freeaddrinfo(addr_server);
        return;
    }

    // set_non_blocking(sock_client);

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

    // Client Loop

    char buffer[MESSAGE_SIZE];
    while (true)
    {
        memset(buffer, 0, MESSAGE_SIZE);
        sprintf(buffer, "Client time: %d\n", time(NULL));
        int buffer_len = strcspn(buffer, "\n");
        buffer[buffer_len] = 0;

        int sent_bytes = send(sock_client, buffer, buffer_len, 0);
        if (sent_bytes != buffer_len)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                report_error("Client send() failed");
                break;
            }
        }
        else
        {
            printf("Client sent: %s\n", buffer);
        }

        memset(buffer, 0, MESSAGE_SIZE);
        int received_bytes = recv(sock_client, buffer, MESSAGE_SIZE, 0);
        if (received_bytes < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                report_error("Client recv() failed");
                break;
            }
        }
        else if (received_bytes == 0)
        {
            printf("Server is down\n");
        }
        else
        {
            printf("Client get response: %s\n", buffer);
        }
    }

    freeaddrinfo(addr_server);
    close(sock_client);
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