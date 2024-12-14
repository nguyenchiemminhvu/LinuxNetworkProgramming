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

#include <openssl/err.h>
#include <openssl/ssl.h>

#define PROTOCOL "tcp"
#define TCP_PORT 4443
#define MESSAGE_SIZE 1024
#define HOST_NAME "localhost"
#define MAX_CONNECTION 100

#define SERVER_CERT_FILE "server.crt"
#define SERVER_KEY_FILE "server.key"

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <client|server>\n", program_name);
}

void report_error(const char* message)
{
    fprintf(stderr, "%ld: Error: %s\n", time(NULL), message);
}

void print_sockaddr_info(sockaddr *sa)
{
    char ip[INET6_ADDRSTRLEN];
    memset(ip, 0, INET6_ADDRSTRLEN);

    void *addr;
    int port;

    if (sa->sa_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)sa;
        addr = &(sin->sin_addr);
        port = ntohs(sin->sin_port);
    }
    else if (sa->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
        addr = &(sin6->sin6_addr);
        port = ntohs(sin6->sin6_port);
    }
    else
    {
        report_error("Unknown address family");
        return;
    }

    if (inet_ntop(sa->sa_family, addr, ip, sizeof(ip)) == NULL)
    {
        report_error("inet_ntop() failed");
        return;
    }

    printf("%s:%d\n", ip, port);
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
    sprintf(port_server, "%d", TCP_PORT);

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

    int optval = 1;
    (void)setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    for (addrinfo* p = addr_server; p != NULL; p = p->ai_next)
    {
        print_sockaddr_info(p->ai_addr);
        rc = bind(sock_server, p->ai_addr, p->ai_addrlen);
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

    SSL_load_error_strings();
    SSL_library_init();

    const SSL_METHOD* p_ssl_method = TLS_server_method();
    SSL_CTX* p_ssl_context = SSL_CTX_new(p_ssl_method);
    if (p_ssl_context == NULL)
    {
        report_error("Server is unable to create SSL context");
        ERR_print_errors_fp(stderr);
        freeaddrinfo(addr_server);
        close(sock_server);
        return;
    }

    rc = SSL_CTX_use_certificate_file(p_ssl_context, SERVER_CERT_FILE, SSL_FILETYPE_PEM);
    if (rc <= 0)
    {
        report_error("Server SSL_CTX_use_certificate_file() failed");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(p_ssl_context);
        freeaddrinfo(addr_server);
        close(sock_server);
        return;
    }

    rc = SSL_CTX_use_PrivateKey_file(p_ssl_context, SERVER_KEY_FILE, SSL_FILETYPE_PEM);
    if (rc <= 0)
    {
        report_error("Server SSL_CTX_use_PrivateKey_file() failed");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(p_ssl_context);
        freeaddrinfo(addr_server);
        close(sock_server);
        return;
    }

    SSL* arr_ssl[MAX_CONNECTION];
    pollfd fds[MAX_CONNECTION];
    memset(arr_ssl, NULL, sizeof(SSL*) * MAX_CONNECTION);
    memset(&fds, 0, sizeof(pollfd) * MAX_CONNECTION);
    fds[0].fd = sock_server;
    fds[0].events = POLLIN;
    arr_ssl[0] = SSL_new(p_ssl_context);
    SSL_set_fd(arr_ssl[0], fds[0].fd);

    for (int i = 1; i < MAX_CONNECTION; i++)
    {
        fds[i].fd = -1;
        arr_ssl[i] = NULL;
    }

    int nfds = 1;

    // Server Loop
    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];
    while (true)
    {
        int activity = poll(fds, MAX_CONNECTION, -1);
        if (activity <= 0)
        {
            report_error("Server poll() failed");
            break;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == sock_server)
                {
                    sockaddr addr_client;
                    socklen_t addr_client_len = sizeof(sockaddr);
                    int sock_client = accept(sock_server, &addr_client, &addr_client_len);
                    if (sock_client > 0)
                    {
                        char ip_client[NI_MAXHOST];
                        char port_client[NI_MAXSERV];
                        rc = getnameinfo(&addr_client, addr_client_len, ip_client, NI_MAXHOST, port_client, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
                        if (rc == 0)
                        {
                            printf("Client %d is connected %s:%s\n", sock_client, ip_client, port_client);
                        }

                        if (nfds < MAX_CONNECTION)
                        {
                            fds[nfds].fd = sock_client;
                            fds[nfds].events = POLLIN;
                            arr_ssl[nfds] = SSL_new(p_ssl_context);
                            SSL_set_fd(arr_ssl[nfds], fds[nfds].fd);

                            rc = SSL_accept(arr_ssl[nfds]);
                            if (rc <= 0)
                            {
                                report_error("Server SSL_accept() failed");
                                ERR_print_errors_fp(stderr);

                                close(fds[nfds].fd);
                                fds[nfds].fd = -1;

                                SSL_free(arr_ssl[nfds]);
                                arr_ssl[nfds] = NULL;
                            }
                            else
                            {
                                set_non_blocking(fds[nfds].fd);
                                nfds++;
                            }
                        }
                    }
                }
                else
                {
                    bool should_disconnect = false;
                    memset(request_buffer, 0, MESSAGE_SIZE);
                    int received_bytes = SSL_read(arr_ssl[i], request_buffer, MESSAGE_SIZE);
                    if (received_bytes <= 0)
                    {
                        should_disconnect = true;
                    }

                    if (strcmp(request_buffer, "quit") == 0 || strcmp(request_buffer, "exit") == 0)
                    {
                        should_disconnect = true;
                    }

                    if (should_disconnect)
                    {
                        printf("Client %d is disconnected\n", fds[i].fd);
                        close(fds[i].fd);
                        SSL_free(arr_ssl[i]);
                        fds[i].fd = fds[nfds - 1].fd;
                        arr_ssl[i] = arr_ssl[nfds - 1];
                        fds[nfds - 1].fd = -1;
                        arr_ssl[nfds - 1] = NULL;
                        nfds--;
                    }
                    else
                    {
                        request_buffer[received_bytes] = 0;
                        printf("Server received %d request: %s\n", fds[i].fd, request_buffer);

                        memset(response_buffer, 0, MESSAGE_SIZE);
                        sprintf(response_buffer, "Server time: %ld", time(NULL));

                        int sent_bytes = SSL_write(arr_ssl[i], response_buffer, strlen(response_buffer));
                        if (sent_bytes <= 0)
                        {
                            report_error("Server SSL_write() failed");
                        }
                    }
                }
            }
        }
    }

    freeaddrinfo(addr_server);
    close(sock_server);

    SSL_CTX_free(p_ssl_context);
    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        if (fds[i].fd > 0)
        {
            close(fds[i].fd);
        }
        
        if (arr_ssl[i] != NULL)
        {
            SSL_shutdown(arr_ssl[i]);
            SSL_free(arr_ssl[i]);
            arr_ssl[i] = NULL;
        }
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

    char port_server[6];
    memset(port_server, 0, 6);
    sprintf(port_server, "%d", TCP_PORT);

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = tcp_proto->p_proto;
    addrinfo* addr_server;
    rc = getaddrinfo(HOST_NAME, port_server, &hints, &addr_server);
    if (rc != 0)
    {
        report_error("Clent getaddrinfo() failed");
        return;
    }

    int sock_client = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
    if (sock_client < 0)
    {
        report_error("Client socket() failed");
        freeaddrinfo(addr_server);
        return;
    }

    for (addrinfo* p = addr_server; p != NULL; p = p->ai_next)
    {
        print_sockaddr_info(p->ai_addr);
        rc = connect(sock_client, p->ai_addr, p->ai_addrlen);
        if (rc == 0)
        {
            break;
        }
    }

    if (rc != 0)
    {
        report_error("Client connect() failed");
        return;
    }

    SSL_load_error_strings();
    SSL_library_init();

    const SSL_METHOD* p_ssl_method = TLS_client_method();
    SSL_CTX* p_ssl_context = SSL_CTX_new(p_ssl_method);
    if (p_ssl_context == NULL)
    {
        report_error("Client is unable to create SSL context");
        ERR_print_errors_fp(stderr);
        freeaddrinfo(addr_server);
        close(sock_client);
        return;
    }

    SSL* p_ssl = SSL_new(p_ssl_context);
    if (p_ssl == NULL)
    {
        report_error("Client SSL_new() failed");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(p_ssl_context);
        freeaddrinfo(addr_server);
        close(sock_client);
        return;
    }

    SSL_set_fd(p_ssl, sock_client);
    rc = SSL_connect(p_ssl);
    if (rc <= 0)
    {
        report_error("Client SSL_connect() failed");
        ERR_print_errors_fp(stderr);
        freeaddrinfo(addr_server);
        close(sock_client);
        SSL_CTX_free(p_ssl_context);
        SSL_shutdown(p_ssl);
        SSL_free(p_ssl);
        return;
    }

    // Client Loop
    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];
    while (true)
    {
        printf("Request: ");
        memset(request_buffer, 0, MESSAGE_SIZE);
        fgets(request_buffer, MESSAGE_SIZE, stdin);
        request_buffer[strcspn(request_buffer, "\r\n")] = 0;

        if (strcmp(request_buffer, "exit") == 0
         || strcmp(request_buffer, "quit") == 0
         || strcmp(request_buffer, "shutdown") == 0)
        {
            break;
        }

        int sent_bytes = SSL_write(p_ssl, request_buffer, strlen(request_buffer));
        if (sent_bytes <= 0)
        {
            report_error("Client SSL_write() failed");
            continue;
        }

        memset(response_buffer, 0, MESSAGE_SIZE);
        int received_bytes = SSL_read(p_ssl, response_buffer, MESSAGE_SIZE);
        if (received_bytes <= 0)
        {
            report_error("Client SSL_read() failed");
            continue;
        }
        response_buffer[received_bytes] = 0;

        printf("Server response: %s\n", response_buffer);
    }

    freeaddrinfo(addr_server);
    close(sock_client);
    SSL_CTX_free(p_ssl_context);
    SSL_shutdown(p_ssl);
    SSL_free(p_ssl);
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