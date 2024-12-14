#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>

int main()
{
    int fd_sock = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
    if (fd_sock == -1)
    {
        printf("failed to create a new socket\n");
    }
    else
    {
        printf("closing newly created socket\n");
        close(fd_sock);
    }
    return 0;
}