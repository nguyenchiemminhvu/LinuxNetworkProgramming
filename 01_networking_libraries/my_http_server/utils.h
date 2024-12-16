#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

void print_sockaddr_info(sockaddr *sa);
void set_socket_nonblocking(int sock);

#endif // UTILS_H