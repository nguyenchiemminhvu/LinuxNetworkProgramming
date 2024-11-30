#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    const char* CPP_HOSTNAME = "cppinstitute.org";

    hostent* host_info = gethostbyname(CPP_HOSTNAME);
    if (host_info == NULL)
    {
        fprintf(stderr, "Error: Could not resolve hostname %s\n", CPP_HOSTNAME);
        return -1;
    }

    printf("Official name: %s\n", host_info->h_name);
    for (char** p_alias = host_info->h_aliases; *p_alias != NULL; p_alias++)
    {
        printf("Alias name %ld: %s\n", p_alias - host_info->h_aliases, *p_alias);
    }

    for (char** p_addr = host_info->h_addr_list; *p_addr != NULL; p_addr++)
    {
        printf("IP address %ld: %s\n", p_addr - host_info->h_addr_list, inet_ntoa(*(in_addr*)*p_addr));
    }

    return 0;
}