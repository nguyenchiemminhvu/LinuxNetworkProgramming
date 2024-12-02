#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <pthread.h>

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

void* server_handle_client(void* arg)
{
    int sock_client = *((int*)arg);
    return NULL;
}

void run_server()
{
    int rc;

    // server loop
    while (true)
    {

    }
}

void run_client()
{
    int rc;

    // client loop
    while (true)
    {

    }
}

int main()
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