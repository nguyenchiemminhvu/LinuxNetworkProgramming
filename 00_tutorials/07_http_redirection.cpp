#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

const char* CPP_HOSTNAME = "httpstat.us";
const int MESSAGE_SIZE = 1024;
const int MAX_REDIRECTION = 5;

enum HTTP_RESULT
{
    HTTP_UNKNOWN = -1,
    HTTP_SUCCESSUL = 200,
    HTTP_REDIRECT = 301,
    HTTP_NOT_FOUND = 404,
};

void report_error(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
}

int perform_http_request(const char* host_name, const char* path, char* response = nullptr, int redirect_count = 0)
{
    if (redirect_count > MAX_REDIRECTION)
    {
        return HTTP_NOT_FOUND;
    }

    protoent* http_proto = getprotobyname("tcp");
    if (http_proto == NULL)
    {
        report_error("TCP protocol is not supported");
        return HTTP_UNKNOWN;
    }

    servent* http_service = getservbyname("http", http_proto->p_name);
    if (http_service == NULL)
    {
        report_error("HTTP service is not available");
        return HTTP_UNKNOWN;
    }

    char http_port[6];
    memset(http_port, 0, 6);
    sprintf(http_port, "%d", ntohs(http_service->s_port));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = http_proto->p_proto;
    addrinfo* http_addr;
    int addr_rc = getaddrinfo(host_name, http_port, &hints, &http_addr);
    if (addr_rc != 0)
    {
        char http_temp[MESSAGE_SIZE];
        memset(http_temp, 0, MESSAGE_SIZE);
        sprintf(http_temp, "Failed to resolve hostname: %s", host_name);
        report_error(http_temp);
        return HTTP_UNKNOWN;
    }

    int http_sock = socket(http_addr->ai_family, http_addr->ai_socktype, http_addr->ai_protocol);
    if (http_sock < 0)
    {
        report_error("socket() failed");
        freeaddrinfo(http_addr);
        return HTTP_UNKNOWN;
    }

    int con_rc = connect(http_sock, http_addr->ai_addr, sizeof(sockaddr));
    if (con_rc != 0)
    {
        report_error("connect() failed");
        freeaddrinfo(http_addr);
        close(http_sock);
        return HTTP_UNKNOWN;
    }

    char http_request[MESSAGE_SIZE];
    memset(http_request, 0, MESSAGE_SIZE);
    sprintf(http_request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host_name);
    printf("HTTP request:\n%s\n", http_request);

    int http_request_len = strlen(http_request);
    int http_sent_bytes = 0;
    while (http_sent_bytes < http_request_len)
    {
        int sent_bytes = send(http_sock, http_request + http_sent_bytes, http_request_len - http_sent_bytes, 0);
        if (sent_bytes <= 0)
        {
            break;
        }
        printf("http sent %d bytes\n", sent_bytes);
        http_sent_bytes += sent_bytes;
    }

    if (http_sent_bytes < http_request_len)
    {
        report_error("Can not send a complete http request");
        freeaddrinfo(http_addr);
        close(http_sock);
        return HTTP_UNKNOWN;
    }

    char http_response[MESSAGE_SIZE];
    memset(http_response, 0, MESSAGE_SIZE);
    int http_received_bytes = 0;
    int received_bytes;
    while ((received_bytes = recv(http_sock, http_response + http_received_bytes, MESSAGE_SIZE - http_received_bytes, 0)) > 0)
    {
        http_received_bytes += received_bytes;
    }

    printf("HTTP received %d bytes\n\n", http_received_bytes);

    if (response != NULL)
    {
        memcpy(response, http_response, http_received_bytes);
    }

    if (strstr(http_response, "301 Moved Permanently") != NULL)
    {
        printf("HTTP is redirected\n\n");
        char* location_start = strstr(http_response, "Location: ");
        if (location_start != NULL)
        {
            location_start += 10;
            char* location_end = strstr(location_start, "\r\n");
            if (location_end != NULL)
            {
                int new_http_host_len = location_end - location_start;
                char new_http_host[MESSAGE_SIZE];
                strncpy(new_http_host, location_start, new_http_host_len);
                new_http_host[new_http_host_len] = '\0';
                return perform_http_request(new_http_host, "/", response, redirect_count + 1);
            }
        }
    }

    freeaddrinfo(http_addr);
    close(http_sock);
    return HTTP_SUCCESSUL;
}

int main()
{
    char http_response[MESSAGE_SIZE];
    memset(http_response, 0, MESSAGE_SIZE);

    (void)perform_http_request(CPP_HOSTNAME, "/200", http_response);
    printf("%s\n", http_response);

    (void)perform_http_request(CPP_HOSTNAME, "/301", http_response);
    printf("%s\n", http_response);

    (void)perform_http_request(CPP_HOSTNAME, "/403", http_response);
    printf("%s\n", http_response);

    (void)perform_http_request(CPP_HOSTNAME, "/404", http_response);
    printf("%s\n", http_response);

    return 0;
}