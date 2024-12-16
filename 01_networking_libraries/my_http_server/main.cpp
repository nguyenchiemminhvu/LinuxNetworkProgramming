#include "http_server.h"
#include <iostream>
#include <stdlib.h>
#include <string>

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <port>\n", program_name);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        print_usage(argv[0]);
        return -1;
    }

    try
    {
        int port = std::stoi(argv[1]);
        HTTPServer server(port);
        server.start();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        print_usage(argv[0]);
    }   

    return 0;
}