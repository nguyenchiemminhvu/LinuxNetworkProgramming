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

#define PROTOCOL "udp"
#define UDP_PORT 45123
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

    protoent* udp_protocol = getprotobyname(PROTOCOL);
    if (udp_protocol == NULL)
    {
        report_error("UDP protocol is not supported");
        return;
    }

    char port_server[6];
    memset(port_server, 0, 6);
    sprintf(port_server, "%d", htons(UDP_PORT));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = udp_protocol->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(NULL, port_server, &hints, &addr_server); // INADDR_ANY
    if (rc != 0)
    {
        report_error("Server can not resolve hostname");
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
        freeaddrinfo(addr_server);
        close(sock_server);
        return;
    }

    printf("Server is ready to receive client requests\n");

    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];
    while (true)
    {
        memset(request_buffer, 0, MESSAGE_SIZE);
        memset(response_buffer, 0, MESSAGE_SIZE);

        sockaddr addr_client;
        socklen_t addr_client_len = sizeof(sockaddr);
        int received_bytes = recvfrom(sock_server, request_buffer, MESSAGE_SIZE, 0, &addr_client, &addr_client_len);
        if (received_bytes <= 0)
        {
            report_error("Server recvfrom() failed");
            continue;
        }

        request_buffer[received_bytes] = '\0';

        char ip_client[NI_MAXHOST];
        char serv_client[NI_MAXSERV];
        rc = getnameinfo(&addr_client, addr_client_len, ip_client, sizeof(ip_client), serv_client, sizeof(serv_client), NI_NUMERICHOST | NI_NUMERICSERV);
        if (rc == 0)
        {
            printf("Server received %s:%s client request: %s\n", ip_client, serv_client, request_buffer);
        }
        else
        {
            printf("Server received unknown client request: %s\n", request_buffer);
        }

        sprintf(response_buffer, "Server received request at %d", time(NULL));
        int response_buffer_len = strlen(response_buffer);
        rc = sendto(sock_server, response_buffer, response_buffer_len, 0, &addr_client, addr_client_len);
        if (rc != response_buffer_len)
        {
            report_error("Server sendto() failed");
        }
    }

    freeaddrinfo(addr_server);
    close(sock_server);

    printf("Server exit with normal status\n");
}

void run_client()
{
    int rc;

    protoent* udp_protocol = getprotobyname(PROTOCOL);
    if (udp_protocol == NULL)
    {
        report_error("UDP protocol is not supported");
        return;
    }

    char port_server[6];
    memset(port_server, 0, 6);
    sprintf(port_server, "%d", htons(UDP_PORT));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = udp_protocol->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(HOST_NAME, port_server, &hints, &addr_server);
    if (rc != 0)
    {
        report_error("Client can not resolve hostname");
        return;
    }

    int sock_client = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (sock_client < 0)
    {
        report_error("Client socket() failed");
        freeaddrinfo(addr_server);
        return;
    }

    // Using sendto() function allows us to specify the destination address and port for each sending packet, no need prior connect() call.
    // Using connect() with UDP socket is still useful sometimes, it is possible to use send() and recv() functions instead of sendto() and recvfrom() functions.
    // for (addrinfo* p_server = addr_server; p_server != NULL; p_server = p_server->ai_next)
    // {
    //     rc = connect(sock_client, p_server->ai_addr, p_server->ai_addrlen);
    //     if (rc == 0)
    //     {
    //         break;
    //     }
    // }

    // if (rc != 0)
    // {
    //     report_error("Client connect() failed");
    //     return;
    // }

    printf("Client is ready to send requests\n");

    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];
    while (true)
    {
        memset(request_buffer, 0, MESSAGE_SIZE);
        memset(response_buffer, 0, MESSAGE_SIZE);

        printf("Enter command: ");
        fgets(request_buffer, MESSAGE_SIZE, stdin);
        request_buffer[strcspn(request_buffer, "\r\n")] = '\0';

        int request_buffer_len = strlen(request_buffer);
        rc = sendto(sock_client, request_buffer, request_buffer_len, 0, addr_server->ai_addr, addr_server->ai_addrlen);
        if (rc != request_buffer_len)
        {
            report_error("Client sendto() failed");
            continue;
        }

        int received_bytes = recvfrom(sock_client, response_buffer, MESSAGE_SIZE, 0, addr_server->ai_addr, &addr_server->ai_addrlen);
        if (received_bytes <= 0)
        {
            report_error("Client recvfrom() failed");
            continue;
        }
        response_buffer[received_bytes] = '\0';
        printf("Client received response: %s\n", response_buffer);

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