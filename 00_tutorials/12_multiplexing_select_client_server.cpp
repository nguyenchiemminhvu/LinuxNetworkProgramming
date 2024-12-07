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
#include <sys/select.h>

#define PROTOCOL "tcp"
#define TCP_PORT 45123
#define MESSAGE_SIZE 1024
#define HOST_NAME "localhost"
#define MAX_CONNECTION 100

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int global_max_fd = 0;

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
    memset(&hints, 0, sizeof(&hints));
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
        return;
    }

    rc = listen(sock_server, MAX_CONNECTION);
    if (rc != 0)
    {
        report_error("Server listen() failed");
        return;
    }

    // Server Loop
    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];

    int fds_client[MAX_CONNECTION];
    memset(fds_client, 0, sizeof(fds_client));

    fd_set read_set;
    while (true)
    {
        FD_ZERO(&read_set);
        FD_SET(sock_server, &read_set);
        global_max_fd = MAX(global_max_fd, sock_server);

        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            if (fds_client[i] > 0)
            {
                FD_SET(fds_client[i], &read_set);
            }

            global_max_fd = MAX(global_max_fd, fds_client[i]);
        }

        int server_activity = select(global_max_fd + 1, &read_set, NULL, NULL, NULL);
        if (server_activity < 0)
        {
            report_error("Server select() failed");
        }

        // check activity on server socket
        if (FD_ISSET(sock_server, &read_set))
        {
            sockaddr addr_client;
            socklen_t addr_client_len = sizeof(sockaddr);
            int sock_client = accept(sock_server, &addr_client, &addr_client_len);
            if (sock_client < 0)
            {
                report_error("Server accept() failed");
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

                for (int i = 0; i < MAX_CONNECTION; i++)
                {
                    if (fds_client[i] == 0)
                    {
                        fds_client[i] = sock_client;
                        break;
                    }
                }
            }
        }
        
        // check activity on connecting clients
        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            if (FD_ISSET(fds_client[i], &read_set))
            {
                memset(request_buffer, 0, MESSAGE_SIZE);
                int received_bytes = recv(fds_client[i], request_buffer, MESSAGE_SIZE, 0);
                if (received_bytes <= 0)
                {
                    report_error("Server recv() failed");
                    close(fds_client[i]);
                    fds_client[i] = 0;
                    continue;
                }

                request_buffer[received_bytes] = 0;
                printf("Server received request: %s\n", request_buffer);

                memset(response_buffer, 0, MESSAGE_SIZE);
                sprintf(response_buffer, "Server time: %d", time(NULL));
                int sent_bytes = send(fds_client[i], response_buffer, strlen(response_buffer), 0);
                if (sent_bytes <= 0)
                {
                    report_error("Server send() failed");
                    close(fds_client[i]);
                    fds_client[i] = 0;
                    continue;
                }

                if (strcmp(request_buffer, "quit") == 0 || strcmp(request_buffer, "exit") == 0)
                {
                    close(fds_client[i]);
                    fds_client[i] = 0;
                }
            }
        }
    }
}

void run_client()
{
    int rc;
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