#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>

int main()
{
    printf("%d\n", getprotobyname("tcp")->p_proto);

    return 0;
}