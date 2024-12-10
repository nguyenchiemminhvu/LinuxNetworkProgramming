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

#include <pthread.h>

#define PROTOCOL "udp"
#define BROADCAST_PORT 5555
#define MESSAGE_SIZE 1024

struct broadcast_t
{
    int fd;
    sockaddr addr_receiver;
    socklen_t addr_receiver_len;
};

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <server|client> <your_nickname>\n", program_name);
}

void report_error(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
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

void set_socket_broadcasting(int socket)
{
    int optval = 1;
    int rc = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
    if (rc == -1)
    {
        report_error("setsockopt(SO_BROADCAST) failed");
    }
}

int setup_broadcast_receiver(broadcast_t& receiver_info)
{
    int rc;

    protoent* udp_proto = getprotobyname(PROTOCOL);
    if (udp_proto == NULL)
    {
        report_error("UDP protocol is not supported");
        return -1;
    }

    char port_broadcast[6];
    memset(port_broadcast, 0, 6);
    sprintf(port_broadcast, "%d", htons(BROADCAST_PORT));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = udp_proto->p_proto;
    hints.ai_flags = AI_PASSIVE;
    addrinfo* addr_broadcast;
    rc = getaddrinfo(NULL, port_broadcast, &hints, &addr_broadcast);
    if (rc != 0)
    {
        report_error("Broadcast address setup failed");
        return -1;
    }

    receiver_info.fd = socket(addr_broadcast->ai_family, addr_broadcast->ai_socktype, addr_broadcast->ai_protocol);
    if (receiver_info.fd < 0)
    {
        report_error("Broadcast receiver socker() failed");
        freeaddrinfo(addr_broadcast);
        return -1;
    }

    set_non_blocking(receiver_info.fd);

    for (addrinfo* p = addr_broadcast; p != NULL; p = p->ai_next)
    {
        rc = bind(receiver_info.fd, p->ai_addr, p->ai_addrlen);
        if (rc == 0)
        {
            receiver_info.addr_receiver = *(p->ai_addr);
            receiver_info.addr_receiver_len = p->ai_addrlen;
            break;
        }
    }

    freeaddrinfo(addr_broadcast);

    if (rc != 0)
    {
        report_error("Broadcast receiver bind() failed");
        return -1;
    }

    return 0;
}

int setup_broadcast_sender(broadcast_t& sender_info)
{
    int rc;

    protoent* udp_proto = getprotobyname(PROTOCOL);
    if (udp_proto == NULL)
    {
        report_error("UDP protocol is not supported");
        return -1;
    }

    char port_broadcast[6];
    memset(port_broadcast, 0, 6);
    sprintf(port_broadcast, "%d", htons(BROADCAST_PORT));

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = udp_proto->p_proto;
    addrinfo* addr_broadcast;
    rc = getaddrinfo(NULL, port_broadcast, &hints, &addr_broadcast);
    if (rc != 0)
    {
        report_error("Broadcast address setup failed");
        return -1;
    }

    sender_info.fd = socket(addr_broadcast->ai_family, addr_broadcast->ai_socktype, addr_broadcast->ai_protocol);
    if (sender_info.fd < 0)
    {
        report_error("Broadcast sender socket() failed");
        freeaddrinfo(addr_broadcast);
        return -1;
    }

    set_socket_broadcasting(sender_info.fd);

    sender_info.addr_receiver = *(addr_broadcast->ai_addr);
    sender_info.addr_receiver_len = addr_broadcast->ai_addrlen;

    freeaddrinfo(addr_broadcast);

    return 0;
}

void run_server()
{
    broadcast_t broadcast_receiver_info;
    if (setup_broadcast_receiver(broadcast_receiver_info) != 0)
    {
        report_error("setup_broadcast_receiver() failed");
        return;
    }

    pollfd fds_receiver[1];
    memset(&fds_receiver, 0, sizeof(fds_receiver));
    fds_receiver[0].fd = broadcast_receiver_info.fd;
    fds_receiver[0].events = POLLIN;

    char buffer[MESSAGE_SIZE];

    printf("Start to listen broadcast messages\n");
    while (true)
    {
        int activity = poll(fds_receiver, 1, -1);
        if (activity < 0)
        {
            report_error("Broadcast receiver thread poll() failed");
            break;
        }

        if (fds_receiver[0].revents & POLLIN)
        {
            memset(buffer, 0, MESSAGE_SIZE);
            int received_bytes = recvfrom(fds_receiver[0].fd, buffer, MESSAGE_SIZE, 0, &broadcast_receiver_info.addr_receiver, &broadcast_receiver_info.addr_receiver_len);
            if (received_bytes <= 0)
            {
                report_error("Broadcast receiver recvfrom() failed");
            }
            else
            {
                printf("%s\n", buffer);
            }
        }
    }

    close(broadcast_receiver_info.fd);
}

void run_client(const char* nick_name)
{
    broadcast_t broadcast_sender_info;
    if (setup_broadcast_sender(broadcast_sender_info) != 0)
    {
        report_error("setup_broadcast_sender() failed");
        return;
    }

    char broadcast_message[MESSAGE_SIZE];
    while (true)
    {
        memset(broadcast_message, 0, MESSAGE_SIZE);
        sprintf(broadcast_message, "%s is active", nick_name);
        int sent_bytes = sendto(broadcast_sender_info.fd, broadcast_message, MESSAGE_SIZE, 0, &broadcast_sender_info.addr_receiver, broadcast_sender_info.addr_receiver_len);
        if (sent_bytes <= 0)
        {
            report_error("Send broadcast message failed");
        }
        sleep(1);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (argc == 2 && strcmp(argv[1], "server") == 0)
    {
        run_server();
    }
    else if (argc == 3 && strcmp(argv[1], "client") == 0)
    {
        run_client(argv[2]);
    }
    else
    {
        print_usage(argv[0]);
        return -1;
    }

    return 0;
}