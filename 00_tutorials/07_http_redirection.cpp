#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

const char* CPP_HOSTNAME = "cppinstitute.org";
const int MESSAGE_SIZE = 1024;

enum HTTP_RESULT
{
    HTTP_UNKNOWN = 0,
    HTTP_SUCCESSUL = 200,
    HTTP_REDIRECT = 301,
    HTTP_NOT_FOUND = 404,
};

void on_func_failure(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

int perform_http_request(const char* host_name, const char* path, char* response = nullptr)
{
    return HTTP_UNKNOWN;
}

int main()
{
    char http_response[MESSAGE_SIZE];
    memset(http_response, 0, MESSAGE_SIZE);

    int http_rc = perform_http_request(CPP_HOSTNAME, "/", http_response);
    switch (http_rc)
    {
    case HTTP_SUCCESSUL:
        printf("%s\n", http_response);
        break;
    }

    return 0;
}