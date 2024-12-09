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
#include <sys/poll.h>

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
    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];

    pollfd fds[MAX_CONNECTION];
    memset(&fds, 0, sizeof(fds));

    fds[0].fd = sock_server;
    fds[0].events = POLLIN;

    int nfds = 1;

    while (true)
    {
        int activity = poll(fds, nfds, -1);
        if (activity < 0)
        {
            report_error("Server poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            sockaddr addr_client;
            socklen_t addr_client_len = sizeof(sockaddr);
            int sock_client = accept(fds[0].fd, &addr_client, &addr_client_len);
            if (sock_client < 0)
            {
                report_error("Server accept() failed");
                continue;
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

                if (nfds < MAX_CONNECTION)
                {
                    fds[nfds].fd = sock_client;
                    fds[nfds].events = POLLIN;
                    nfds++;
                }
            }
        }

        for (int i = 1; i >= 1 && i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                memset(request_buffer, 0, MESSAGE_SIZE);
                int received_bytes = recv(fds[i].fd, request_buffer, MESSAGE_SIZE, 0);
                if (received_bytes <= 0)
                {
                    if (received_bytes < 0)
                    {
                        report_error("Server recv() failed");
                    }
                    else
                    {
                        printf("Client %d disconnected\n", fds[i].fd);
                    }

                    close(fds[i].fd);
                    fds[i].fd = fds[nfds - 1].fd;
                    nfds--;
                    i--;
                }
                else
                {
                    request_buffer[received_bytes] = 0;
                    printf("Server received %d request: %s\n", fds[i].fd, request_buffer);

                    memset(response_buffer, 0, MESSAGE_SIZE);
                    sprintf(response_buffer, "Server time: %ld", time(NULL));

                    bool should_disconnect = false;
                    int sent_bytes = send(fds[i].fd, response_buffer, strlen(response_buffer), 0);
                    if (sent_bytes <= 0)
                    {
                        should_disconnect = true;
                    }

                    if (strcmp(request_buffer, "quit") == 0 || strcmp(request_buffer, "exit") == 0)
                    {
                        should_disconnect = true;
                    }

                    if (should_disconnect)
                    {
                        printf("Client %d disconnect\n", fds[i].fd);
                        close(fds[i].fd);
                        fds[i].fd = fds[nfds - 1].fd;
                        nfds--;
                        i--;
                    }
                }
            }
        }
    }

    printf("Server exit with normal status");
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