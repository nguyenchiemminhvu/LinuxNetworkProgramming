#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    servent* service_info;
    service_info = getservbyname("http", getprotobyname("tcp")->p_name);
    if (service_info == NULL)
    {
        fprintf(stderr, "Error: Can not resolve service http\n");
        return -1;
    }

    printf("Service name: %s\n", service_info->s_name);
    printf("Port number: %d\n", ntohs(service_info->s_port));
    printf("Protocol: %s\n", service_info->s_proto);

    return 0;
}