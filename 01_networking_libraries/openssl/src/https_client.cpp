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

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <hostname> <port>\n", program_name);
}

void report_error(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
}

void perform_https_request(char* hostname, int port)
{
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        print_usage(argv[0]);
        return -1;
    }

    perform_https_request(argv[1], atoi(argv[2]));

    return 0;
}