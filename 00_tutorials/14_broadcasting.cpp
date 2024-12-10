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
#define BROADCAST_ADDR "255.255.255.255"
#define BROADCAST_PORT 5555
#define MESSAGE_SIZE 1024

struct broadcast_t
{
    int fd;
    sockaddr_in addr_receiver;
    socklen_t addr_receiver_len;
};

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <your_nickname>\n", program_name);
}

void report_error(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
}

int setup_broadcast_receiver(broadcast_t& receiver_info)
{
    receiver_info.fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (receiver_info.fd < 0)
    {
        report_error("socket() failed for receiver");
        return -1;
    }

    int optval = 1;
    if (setsockopt(receiver_info.fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        report_error("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    receiver_info.addr_receiver.sin_family = AF_INET;
    receiver_info.addr_receiver.sin_port = htons(BROADCAST_PORT);
    receiver_info.addr_receiver.sin_addr.s_addr = htonl(INADDR_ANY);
    receiver_info.addr_receiver_len = sizeof(receiver_info.addr_receiver);

    if (bind(receiver_info.fd, (sockaddr *)&receiver_info.addr_receiver, receiver_info.addr_receiver_len) < 0)
    {
        report_error("bind() failed for receiver");
        return -1;
    }

    return 0;
}

int setup_broadcast_sender(broadcast_t& sender_info)
{
    sender_info.fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender_info.fd < 0)
    {
        report_error("socket() failed for sender");
        return -1;
    }

    int optval = 1;
    if (setsockopt(sender_info.fd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1)
    {
        report_error("setsockopt(SO_BROADCAST) failed");
        return -1;
    }

    sender_info.addr_receiver.sin_family = AF_INET;
    sender_info.addr_receiver.sin_port = htons(BROADCAST_PORT);
    inet_pton(AF_INET, BROADCAST_ADDR, &sender_info.addr_receiver.sin_addr);
    sender_info.addr_receiver_len = sizeof(sender_info.addr_receiver);

    return 0;
}

void* broadcast_receiver_thread_func(void* arg)
{
    broadcast_t broadcast_receiver_info;
    if (setup_broadcast_receiver(broadcast_receiver_info) != 0)
    {
        report_error("setup_broadcast_receiver() failed");
        return NULL;
    }

    char buffer[MESSAGE_SIZE];

    printf("Start to listen broadcast messages\n");
    while (true)
    {
        memset(buffer, 0, MESSAGE_SIZE);
        int received_bytes = recvfrom(broadcast_receiver_info.fd, buffer, MESSAGE_SIZE, 0, (sockaddr*)&broadcast_receiver_info.addr_receiver, &broadcast_receiver_info.addr_receiver_len);
        if (received_bytes <= 0)
        {
            report_error("Broadcast receiver recvfrom() failed");
        }
        else
        {
            printf("Received broadcast message: %s\n", buffer);
        }
    }

    close(broadcast_receiver_info.fd);

    return NULL;
}

void* broadcast_sender_thread_func(void* arg)
{
    char* nick_name = (char*)arg;

    broadcast_t broadcast_sender_info;
    if (setup_broadcast_sender(broadcast_sender_info) != 0)
    {
        report_error("setup_broadcast_sender() failed");
        return NULL;
    }

    char broadcast_message[MESSAGE_SIZE];
    while (true)
    {
        memset(broadcast_message, 0, MESSAGE_SIZE);
        sprintf(broadcast_message, "%s is active", nick_name);
        int sent_bytes = sendto(broadcast_sender_info.fd, broadcast_message, MESSAGE_SIZE, 0, (sockaddr*)&broadcast_sender_info.addr_receiver, broadcast_sender_info.addr_receiver_len);
        if (sent_bytes <= 0)
        {
            report_error("Send broadcast message failed");
        }
        sleep(1);
    }

    close(broadcast_sender_info.fd);

    return NULL;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        print_usage(argv[0]);
        return -1;
    }

    pthread_t broadcast_receiver_thread;
    pthread_create(&broadcast_receiver_thread, NULL, broadcast_receiver_thread_func, NULL);
    pthread_detach(broadcast_receiver_thread);

    pthread_t broadcast_sender_thread;
    pthread_create(&broadcast_sender_thread, NULL, broadcast_sender_thread_func, argv[1]);
    pthread_detach(broadcast_sender_thread);

    char buffer_stdin[MESSAGE_SIZE];
    while (true)
    {
        printf("User input: ");
        memset(buffer_stdin, 0, MESSAGE_SIZE);
        fgets(buffer_stdin, MESSAGE_SIZE, stdin);
        buffer_stdin[strcspn(buffer_stdin, "\r\n")] = 0;

        if (strcmp(buffer_stdin, "exit") == 0
         || strcmp(buffer_stdin, "quit") == 0)
        {
            break;
        }
    }

    return 0;
}