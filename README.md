- [Introduction](#introduction)
- [Sockets](#sockets)
  - [Type of sockets](#type-of-sockets)
  - [Addressing sockets](#addressing-sockets)
  - [Socket APIs](#socket-apis)
    - [getprotobyname()](#getprotobyname)
    - [getservbyname()](#getservbyname)
    - [getaddrinfo()](#getaddrinfo)
    - [htonl(), htons(), ntohl(), ntohs()](#htonl-htons-ntohl-ntohs)
    - [socket()](#socket)
    - [setsockopt()](#setsockopt)
    - [bind()](#bind)
    - [listen()](#listen)
    - [accept()](#accept)
    - [connect()](#connect)
    - [recv()](#recv)
    - [send()](#send)
    - [close()](#close)
- [Programming Client-Server Models](#programming-client-server-models)
  - [Client-Server Architecture](#client-server-architecture)
  - [Simple HTTP Client](#simple-http-client)
    - [Include necessary headers](#include-necessary-headers)
    - [Define some constants](#define-some-constants)
    - [Define Helper functions](#define-helper-functions)
    - [Get TCP Protocol](#get-tcp-protocol)
    - [Get Port of HTTP service](#get-port-of-http-service)
    - [Convert Port to Network Byte Order](#convert-port-to-network-byte-order)
    - [Resolve Host Name](#resolve-host-name)
    - [Create A Socket](#create-a-socket)
    - [Connect to HTTP server](#connect-to-http-server)
    - [Send HTTP Request](#send-http-request)
    - [Receive HTTP Response](#receive-http-response)
    - [Clean Up](#clean-up)
  - [Simple TCP-Based Client-Server](#simple-tcp-based-client-server)
    - [Necessary Headers And Macros](#necessary-headers-and-macros)
    - [Utility Functions](#utility-functions)
    - [Setup Server](#setup-server)
    - [Setup Client](#setup-client)
    - [Main Function](#main-function)
  - [Multithread TCP-Based Client-Server](#multithread-tcp-based-client-server)
    - [Setup Server with multithreading](#setup-server-with-multithreading)
  - [Simple UDP-Based Client-Server](#simple-udp-based-client-server)
    - [Necessary Headers And Macros](#necessary-headers-and-macros-1)
    - [Setup Server](#setup-server-1)
    - [Setup Client](#setup-client-1)
  - [Advanced Techniques](#advanced-techniques)
    - [Non-blocking sockets](#non-blocking-sockets)
    - [Synchronous I/O Multiplexing with select()](#synchronous-io-multiplexing-with-select)
    - [Synchronous I/O Multiplexing with poll()](#synchronous-io-multiplexing-with-poll)
    - [Broadcasting Messages](#broadcasting-messages)
  - [Create A Simple HTTP Server](#create-a-simple-http-server)
    - [Demo](#demo)
- [Networking Libraries](#networking-libraries)
  - [Using libcurl](#using-libcurl)
    - [curl command-line examples](#curl-command-line-examples)
    - [Basic Curl](#basic-curl)
    - [Curl Multiple Handles](#curl-multiple-handles)
    - [Curl Multithreading](#curl-multithreading)
  - [Secure Networking with OpenSSL](#secure-networking-with-openssl)
    - [A HTTPS Client](#a-https-client)
    - [Secure Client-Server](#secure-client-server)
- [Conclusion](#conclusion)


# Introduction

In my opinion, Linux network programming, especially socket programming, isn’t that difficult. However, learning this topic on your own can be challenging because many online resources are unclear, and sample codes often only cover the basics. You might find yourself unsure of what to do next. That's why I created this tutorial. It aims to give you clear guidelines and plenty of examples to help you understand better.

Linux network programming deals with the interaction between processes using network interfaces. It enables interprocess communication (```IPC```), allowing data exchange between processes running on the same machine or on different machines connected over a network.

The foundation of Linux network programming lies in the use of sockets, a universal API designed for interprocess communication. Sockets originated from BSD Unix in 1983 and were later standardized by POSIX, making them a cornerstone of modern networking.

# Sockets

A socket is an endpoint for communication. Think of it as a door through which data flows in and out of a process. Processes use sockets to send and receive messages, enabling seamless ```IPC```.

Sockets were initially designed to support two domains:

**Unix Domain (Unix)**: Used for communication between processes within the same operating system.

**Internet Domain (INET)**: Used for communication between processes on different systems connected via a TCP/IP network.

Unix domain sockets are used for ```IPC``` within the same operating system. They are faster than ```INET``` sockets because they don't require network protocol overhead. Instead of IP addresses, Unix domain sockets use file system paths for addressing.

```INET``` domain sockets are used for communication between processes on different systems connected over a network. These sockets rely on the ```TCP/IP``` protocol stack, which ensures data integrity and delivery.

Two common protocols used with ```INET``` domain sockets are:

**TCP (Transmission Control Protocol)**: Provides reliable, ordered, and error-checked delivery of data.

**UDP (User Datagram Protocol)**: Provides fast, connectionless data transmission without guarantees of delivery.

## Type of sockets

The BSD socket API supports several types of sockets, which determine how data is transmitted between processes:

**Stream Sockets (SOCK_STREAM)**: These provide a reliable, connection-oriented communication protocol. Data is sent and received as a continuous stream of bytes. Typically used with ```TCP``` (Transmission Control Protocol).

**Datagram Sockets (SOCK_DGRAM)**: These provide a connectionless communication protocol. Data is sent in discrete packets, and delivery isn't guaranteed. Typically used with ```UDP``` (User Datagram Protocol).

**Raw Sockets (SOCK_RAW)**: These allow processes to access lower-level network protocols directly, bypassing the standard ```TCP``` or ```UDP``` layers. Useful for custom protocol implementations or network monitoring tools.

## Addressing sockets

In the INET domain, sockets are identified by two components:

**IP Address**: A 32-bit number (```IPv4```) or a 128-bit number (```IPv6```) that uniquely identifies a device on a network. IPv4 addresses are often represented in dotted decimal notation, such as ```192.168.1.1```.

**Port Number**: A 16-bit number that identifies a specific service or application on the device. For example, web servers typically use port 80 (```HTTP```) or 443 (```HTTPS```).

Check some of well-known services in Linux system via ```/etc/services``` file. Ports under 1024 are often considered special, and usually require special OS privileges to use.

```
worker@7e4a84e41875:~/study_workspace/LinuxNetworkProgramming$ cat /etc/services
tcpmux          1/tcp                           # TCP port service multiplexer
echo            7/tcp
echo            7/udp
...
ftp             21/tcp
fsp             21/udp          fspd
ssh             22/tcp                          # SSH Remote Login Protocol
telnet          23/tcp
smtp            25/tcp          mail
...
http            80/tcp          www             # WorldWideWeb HTTP
...
```

## Socket APIs

### getprotobyname()

```
#include <netdb.h>

struct protoent *getprotobyname(const char *name);
```

Sample usage:

```
struct protoent *proto;
proto = getprotobyname("tcp");
if (proto)
{
    printf("Protocol number for TCP: %d\n", proto->p_proto);
}
```

**Description**: ```getprotobyname()``` returns a ```protoent``` structure for the given protocol name, which contains information about the protocol.

### getservbyname()

```
#include <netdb.h>

struct servent *getservbyname(const char *name, const char *proto);
```

Sample usage:

```
struct servent *serv;
serv = getservbyname("http", "tcp");
if (serv)
{
    printf("Port number for HTTP: %d\n", ntohs(serv->s_port));
}
```

**Description**: ```getservbyname()``` returns a ```servent``` structure for the given service name and protocol, which contains information about the service.

### getaddrinfo()

```
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};
 
int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);
```

Struct ```addrinfo``` has the pointer to struct ```sockaddr``` which is used in many socket functions.

Sample usage:

```
int status;
struct addrinfo hints;
struct addrinfo *servinfo;  // will point to the results
 
memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
 
if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0)
{
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
}
 
// servinfo now points to a linked list of 1 or more struct addrinfos
 
// ... do everything until you don't need servinfo anymore ....
 
freeaddrinfo(servinfo); // free the linked-list
```

**Description**: ```getaddrinfo()``` is used to get a list of address structures for the specified node and service, which can be used to create and connect sockets.

### htonl(), htons(), ntohl(), ntohs()

```
#include <arpa/inet.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```

Sample usage:

```
uint32_t host_port = 8080;
uint32_t net_port = htonl(host_port);
printf("Network byte order: 0x%x\n", net_port);
```

**Description**: These functions convert values between host and network byte order. ```htonl()``` and ```htons()``` convert from host to network byte order, while ```ntohl()``` and ```ntohs()``` convert from network to host byte order.

```htons(uint16_t hostshort)```: Converts a 16-bit number from host byte order to network byte order.

```htonl(uint32_t hostlong)```: Converts a 32-bit number from host byte order to network byte order.

```ntohs(uint16_t netshort)```: Converts a 16-bit number from network byte order to host byte order.

```ntohl(uint32_t netlong)```: Converts a 32-bit number from network byte order to host byte order.

**What is Network Byte Order?**

Network byte order is a standardized way of arranging the bytes of multi-byte data types (like integers) in network communication. Different CPU architecture may process data in different orders, we called it **"endianness"**.

> Big-endian (BE): Stores the most significant byte (the “big end”) first. This means that the first byte (at the lowest memory address) is the largest, which makes the most sense to people who read left to right
 
> Little-endian (LE): Stores the least significant byte (the “little end”) first. This means that the first byte (at the lowest memory address) is the smallest, which makes the most sense to people who read right to left.

Data transferred in network is always Big-endian order.

Data sending from host machine is called Host-byte order, could be big or little endian. Using the functions above ensure proper communication between systems.

### socket()

```
#include <sys/types.h>
#include <sys/socket.h>
 
int socket(int domain, int type, int protocol);
```

Sample usage:

```
int s;
struct addrinfo hints, *res;
 
getaddrinfo("www.example.com", "http", &hints, &res);
 
s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
if (s == -1)
{
    perror("socket");
    exit(1);
}
```

**Description**: ```socket()``` creates a new socket and returns a file descriptor for it.

### setsockopt()

```
#include <sys/types.h>
#include <sys/socket.h>

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
```

Sample usage:

```
int sockfd; // Assume sockfd is a valid socket descriptor
int optval = 1;
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
{
    perror("setsockopt");
    exit(1);
}
```

**Description**: ```setsockopt()``` sets options on a socket, such as enabling the reuse of local addresses.

### bind()

```
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

Sample usage:

```
int sockfd; // Assume sockfd is a valid socket descriptor
struct sockaddr_in my_addr;
my_addr.sin_family = AF_INET;
my_addr.sin_port = htons(3490);
my_addr.sin_addr.s_addr = INADDR_ANY;

if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
{
    perror("bind");
    exit(1);
}
```

**Description**: ```bind()``` assigns a local address to a socket.

### listen()

```
#include <sys/types.h>
#include <sys/socket.h>

int listen(int sockfd, int backlog);
```

Sample usage:

```
int sockfd; // Assume sockfd is a valid socket descriptor
if (listen(sockfd, 10) == -1)
{
    perror("listen");
    exit(1);
}
```

**Description**: ```listen()``` marks a socket as a passive socket that will be used to accept incoming connection requests.

### accept()

```
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

Sample usage:

```
int sockfd; // Assume sockfd is a valid socket descriptor
struct sockaddr_storage their_addr;
socklen_t addr_size = sizeof(their_addr);
int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
if (new_fd == -1)
{
    perror("accept");
    exit(1);
}
```

**Description**: ```accept()``` accepts a connection on a socket. If the server wants to send responding data back to client, it will send data to the returned fd.

### connect()

```
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

Sample usage:

```
int sockfd; // Assume sockfd is a valid socket descriptor
struct addrinfo hints, *res;

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;

if (getaddrinfo("www.example.com", "http", &hints, &res) != 0)
{
    fprintf(stderr, "getaddrinfo error\n");
    exit(1);
}

if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
{
    perror("connect");
    exit(1);
}
```

**Description**: ```connect()``` initiates a connection on a socket.

### recv()

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

Sample usage:

```
int sockfd; // Assume sockfd is a valid socket descriptor
char buf[100];
ssize_t bytes_received = recv(sockfd, buf, sizeof(buf), 0);
if (bytes_received == -1)
{
    perror("recv");
    exit(1);
}
else
{
    printf("Received %zd bytes\n", bytes_received);
}
```

**Description**: ```recv()``` receives data from a socket.

### send()

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```

Sample usage:

```
int sockfd; // Assume sockfd is a valid socket descriptor
char *msg = "Hello, World!";
ssize_t bytes_sent = send(sockfd, msg, strlen(msg), 0);
if (bytes_sent == -1)
{
    perror("send");
    exit(1);
}
else
{
    printf("Sent %zd bytes\n", bytes_sent);
}
```

**Description**: ```send()``` sends data to a socket.

### close()

```
#include <unistd.h>

int close(int fd);
```

Sample usage:

```
close(sockfd);
```

**Description**: ```close()``` closes a file descriptor, so that it no longer refers to any file and may be reused.

# Programming Client-Server Models

## Client-Server Architecture

The client-server model is a way of organizing networked computers where one computer (the client) requests services or resources from another computer (the server). The server provides these services or resources to the client.

## Simple HTTP Client

Here are our goals:

- we want to write a program which gets the address of a WWW site (e.g. ```httpstat.us```) as the argument and fetches the document.
- the program outputs the document to stdout;
- the program uses TCP to connect to the ```HTTP``` server.

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/07_http_redirection.c) for a complete source code.

![HTTP Connection](https://raw.githubusercontent.com/nguyenchiemminhvu/LinuxNetworkProgramming/refs/heads/main/http_connection.png)

### Include necessary headers

```
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
```

```unistd.h```: Provides access to the POSIX operating system API, including file descriptors and the close() function.

```stdio.h```: Standard I/O library for input/output operations (e.g., printf, fprintf).

```stdlib.h```: Standard library for memory management (malloc, free) and program control (exit).

```string.h```: Provides string manipulation functions like memset, strlen, etc.

```arpa/inet.h```: Functions for manipulating IP addresses, such as ntohs and inet_ntoa.

```netdb.h```: Functions for network database operations, such as getaddrinfo, getprotobyname, and getservbyname.

```sys/socket.h```: Defines socket-related functions like socket, connect, send, and recv.

### Define some constants

```
const char* CPP_HOSTNAME = "httpstat.us";
const int MESSAGE_SIZE = 1024;
```

```CPP_HOSTNAME```: The hostname of the remote server the program will connect to.

```MESSAGE_SIZE```: The size of the buffer used for sending and receiving data (1 KB in this case).

### Define Helper functions

```
void on_func_failure(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}
```

A helper function to handle errors. When a function fails, this function is called with an error message.

```fprintf(stderr, ...)```: Prints the error message to the standard error output.

```exit(EXIT_FAILURE)```: Terminates the program with a failure status code.

### Get TCP Protocol

```
struct protoent* p_proto_ent = getprotobyname("tcp");
if (p_proto_ent == NULL)
{
    on_func_failure("TCP protocol is not available");
}
```

```getprotobyname("tcp")```: Retrieves the protocol entry for "tcp" (Transmission Control Protocol). The function returns a protoent structure that contains protocol information.

If it returns NULL, the program exits using ```on_func_failure``` because TCP is required for the connection.

### Get Port of HTTP service

```
servent* p_service_ent = getservbyname("http", p_proto_ent->p_name);
if (p_service_ent == NULL)
{
    on_func_failure("HTTP service is not available");
}
```

```getservbyname("http", p_proto_ent->p_name)```: Retrieves information about the "http" service, including the port number (usually 80 for HTTP).

If getservbyname fails, the program exits. This function ensures the port number for HTTP is available.

### Convert Port to Network Byte Order

```
char port_buffer[6];
memset(port_buffer, 0, sizeof(port_buffer));
sprintf(port_buffer, "%d", ntohs(p_service_ent->s_port));
```

Port Conversion: The port number from ```getservbyname``` is in network byte order (big-endian). ```ntohs``` converts it to host byte order (little-endian, on most systems).

```sprintf```: Converts the port number to a string (stored in port_buffer), which is required by getaddrinfo.

### Resolve Host Name

```
struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_protocol = p_proto_ent->p_proto;
hints.ai_socktype = SOCK_STREAM;

struct addrinfo* server_addr;
int rc = getaddrinfo(CPP_HOSTNAME, port_buffer, &hints, &server_addr);
if (rc != 0)
{
    on_func_failure("Failed to resolve hostname");
}
```

```addrinfo```: A structure that holds information for socket creation.

```ai_family = AF_INET```: Specifies IPv4 addresses.

```ai_socktype = SOCK_STREAM```: Specifies a TCP connection.

```ai_protocol = p_proto_ent->p_proto```: Ensures the protocol is TCP.

```getaddrinfo```: Resolves the hostname (cppinstitute.org) and port (80) into an address that can be used for connecting.

If getaddrinfo fails, the program exits.

### Create A Socket

```
int sock_fd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol);
if (sock_fd < 0)
{
    freeaddrinfo(server_addr);
    on_func_failure("socket() failed");
}
```

```socket()```: Creates a new socket for communication.

The arguments specify the address family, socket type, and protocol (IPv4, TCP).

If the socket creation fails, the program exits.

### Connect to HTTP server

```
rc = connect(sock_fd, server_addr->ai_addr, sizeof(struct sockaddr));
if (rc != 0)
{
    freeaddrinfo(server_addr);
    on_func_failure("connect() failed");
}
```

```connect()```: Initiates a connection to the remote server using the socket.

If connect fails, the program cleans up allocated resources and exits.

### Send HTTP Request

```
char http_request[MESSAGE_SIZE];
memset(http_request, 0, MESSAGE_SIZE);
sprintf(http_request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", CPP_HOSTNAME);

int http_request_len = strlen(http_request);
int sent_bytes = 0;
while (sent_bytes < http_request_len)
{
    int sent_rc = send(sock_fd, http_request + sent_bytes, http_request_len - sent_bytes, 0);
    printf("sent %d bytes\n", sent_rc);
    sent_bytes += sent_rc;
}
```

A conversation with the HTTP server consists of requests (sent by the client) and responses (sent by the server).

To get a root document from a site named www.site.com, the client should send the request to the server:

```
GET / HTTP / 1.1 \ r \ n Host: www.site.com \ r \ n Connection:close \ r \ n \ r \ n
```

The request consists of:

- a line containing a request name (```GET```) followed by the name of the resource the client wants to receive; the root documents is specified as a single slash (```/```); the line must also include the HTTP protocol version (```HTTP/1.1```), and must end with the ```\r\n``` characters; note: all the lines must be ended in the same way;
- a line containing the name of the site (```www.site.com```) preceded by the parameter name (Host:)
- a line containing the parameter named Connection: along with its value, close forces the server to close the connection after the first request is served; it will simplify our client’s code;
- an empty line is the request’s terminator.

If the request is correct, the server’s response will begin with a more or less similar header.

```
HTTP/1.1 200 OK
Content-Type: text/plain
Date: Thu, 05 Dec 2024 07:07:58 GMT
Server: Kestrel
Set-Cookie: ARRAffinity=b3b03edd65273a52d0e5a4a4995ddf09acfbb7f67adccaf277d300c0a375ea34;Path=/;HttpOnly;Domain=httpstat.us
Request-Context: appId=cid-v1:3548b0f5-7f75-492f-82bb-b6eb0e864e53
X-RBT-CLI: Name=LGEVN-Hanoi-ACC-5080M-A; Ver=9.14.2b;
Connection: close
Content-Length: 6

200 OK
```

### Receive HTTP Response

```
char http_response[MESSAGE_SIZE];
memset(http_response, 0, MESSAGE_SIZE);
int received_bytes = 0;
while (1 == 1)
{
    int received_rc = recv(sock_fd, http_response + received_bytes, MESSAGE_SIZE - received_bytes, 0);
    printf("Received %d bytes\n", received_rc);
    received_bytes += received_rc;
}
```

```recv()```: Receives the server's response in chunks and appends it to the http_response buffer.

When ```recv()``` returns 0 or a negative value, it indicates the server has closed the connection or an error occurred.

### Clean Up

```
close(sock_fd);
freeaddrinfo(server_addr);
```

```close()```: Closes the socket, releasing system resources.

```freeaddrinfo()```: Frees the memory allocated by getaddrinfo.

## Simple TCP-Based Client-Server

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/08_simple_client_server.c) for a complete source code.

![TCP-Based Client-Server](https://raw.githubusercontent.com/nguyenchiemminhvu/LinuxNetworkProgramming/refs/heads/main/tcp_based_client_server.png)

### Necessary Headers And Macros

```
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define PROTOCOL "tcp"
#define TCP_PORT 45123
#define MESSAGE_SIZE 1024
#define HOST_NAME "localhost"
```

**Headers:**

```unistd.h```: Provides POSIX API functions, e.g., close.

```stdio.h```: For input/output operations, e.g., printf, fprintf.

```time.h```: To get the current time using time().

```string.h```: For string operations, e.g., strcmp, memset.

```stdlib.h```: For memory allocation and process control.

```arpa/inet.h```: For socket-related functions and data structures.

```netdb.h```: To resolve hostnames using getaddrinfo and gethostbyname.

```sys/socket.h```: Core socket programming functions, e.g., socket, connect, bind.

```sys/stat.h```: For file and directory operations.

**Macros:**

```PROTOCOL```: Defines the protocol as tcp.

```TCP_PORT```: The port number the server and client use for communication.

```MESSAGE_SIZE```: Maximum size for messages sent/received.

```HOST_NAME```: Default hostname, set to localhost.

### Utility Functions

```
void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <client|server>\n", program_name);
}
```

Prints a usage guide, showing how to execute the program. Example usage:

```./program_name client```

or

```./program_name server.```

```
void report_error(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
}
```

Prints error messages to stderr.

### Setup Server

**Choose protocol and resolve server address**

```
struct protoent* tcp_proto = getprotobyname(PROTOCOL);
```

Retrieves the protocol structure for the ```"tcp"``` protocol using ```getprotobyname()```.

```
char server_port[6];
memset(server_port, 0, 6);
sprintf(server_port, "%d", htons(TCP_PORT));

struct addrinfo addr_hints;
memset(&addr_hints, 0, sizeof(addr_hints));
addr_hints.ai_family = AF_INET;
addr_hints.ai_socktype = SOCK_STREAM;
addr_hints.ai_protocol = tcp_proto->p_proto;

struct addrinfo* addr_server;
rc = getaddrinfo(NULL, server_port, &addr_hints, &addr_server);
```

Converts the TCP port into network byte order using ```htons()```.

Initializes an addrinfo structure to specify connection parameters:
- ai_family = AF_INET: IPv4.
- ai_socktype = SOCK_STREAM: TCP socket.

Resolves the server's address information using ```getaddrinfo()```.

**Create server socket**

```
int sock_server = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
```

Creates a socket using the ```socket()``` function.


```
int sock_server_opt = 1;
rc = setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE, &sock_server_opt, sizeof(sock_server_opt));
```

Configures socket options:

```SO_REUSEADDR```: Allows the server to reuse the same port.

```SO_KEEPALIVE```: Keeps the connection alive.

**Bind socket to address and start to listen**

```
for (addrinfo* p_server = addr_server; p_server != NULL; p_server = p_server->ai_next)
{
    rc = bind(sock_server, p_server->ai_addr, p_server->ai_addrlen);
    if (rc == 0)
    {
        break;
    }
}
```

Binds the socket to the resolved address using ```bind()``` function.

Iterates over potential addresses (```addrinfo``` list) until successful.

```
rc = listen(sock_server, 3);
```

Starts listening for incoming client connections with a backlog of 3.

**Server Loop - Accept incoming client connection**

```
struct sockaddr addr_client;
socklen_t addr_len = sizeof(addr_client);
sock_client = accept(sock_server, (struct sockaddr*)&addr_client, &addr_len);
```

Accepts incoming client connections using ```accept()``` function.

**Server Loop - Receiving requests**

```
int received_bytes = recv(sock_client, request_buffer, MESSAGE_SIZE, 0);
```

Reads data from the client using ```recv()``` function.

**Server Loop - Processing requests**

```
if (strcmp(request_buffer, "exit") == 0
|| strcmp(request_buffer, "quit") == 0
|| strcmp(request_buffer, "shutdown") == 0)
{
    sprintf(response_buffer, "OK");
    rc = send(sock_client, response_buffer, MESSAGE_SIZE, 0);
    close(sock_client);
    break;
}
else if (strcmp(request_buffer, "time") == 0)
{
    sprintf(response_buffer, "%d", time(NULL));
    rc = send(sock_client, response_buffer, MESSAGE_SIZE, 0);
}
else
{
    sprintf(response_buffer, "Unknown request");
    rc = send(sock_client, response_buffer, MESSAGE_SIZE, 0);
}
```

Handles specific commands:

```time```: Sends the current time.

```exit, quit, shutdown```: Terminates the connection.

```Other inputs```: Responds with "Unknown request".

### Setup Client

**Choose protocol and resolve server address**

```
struct protoent* tcp_proto = getprotobyname(PROTOCOL);

struct addrinfo addr_hints;
memset(&addr_hints, 0, sizeof(addr_hints));
addr_hints.ai_family = AF_INET;
addr_hints.ai_socktype = SOCK_STREAM;
addr_hints.ai_protocol = tcp_proto->p_proto;

struct addrinfo* addr_server;
rc = getaddrinfo(HOST_NAME, server_port, &addr_hints, &addr_server);
```

Resolves the server's address.

**Create client socket and connect to server**

```
int sock_client = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);

for (addrinfo* p_server = addr_server; p_server != NULL; p_server = p_server->ai_next)
{
    rc = connect(sock_client, p_server->ai_addr, p_server->ai_addrlen);
    if (rc == 0)
    {
        break;
    }
}
```

Creates a socket and connects to the server.

Iterates over potential addresses (```addrinfo``` list) until successful.

**Client Loop - Send request and wait for response**

```
fgets(request_buffer, MESSAGE_SIZE, stdin);
request_buffer[strcspn(request_buffer, "\n")] = 0;
send(sock_client, request_buffer, strlen(request_buffer), 0);
recv(sock_client, response_buffer, MESSAGE_SIZE, 0);
```

Sends user input to the server and waits for a response.

### Main Function

```
if (strcmp(argv[1], "server") == 0)
{
    run_server();
}
else if (strcmp(argv[1], "client") == 0)
{
    run_client();
}
```

Determines whether the program will run as a server or client based on the command-line argument.

## Multithread TCP-Based Client-Server

The provided C program is a simple implementation of a TCP client-server application and works well for basic use cases. However, it has several limitations, particularly on the server-side: it can only handle one client connection at a time. While it processes a request from one client, other clients are left waiting.

**Improvement:**

Use multithreading to handle multiple clients concurrently. Each client connection can be assigned to a separate thread, allowing the server to process multiple requests simultaneously.

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/09_multithread_client_server.c) for a complete source code.

### Setup Server with multithreading

The setup of Server socket and connection is the same as before. But in the Server Loop, each client connection will be handled in a detached thread.

```
int* p_sock_client = (int*)calloc(1, sizeof(int));
*p_sock_client = sock_client;
pthread_t client_thread;
rc = pthread_create(&client_thread, NULL, server_handle_client, p_sock_client);

rc = pthread_detach(client_thread);
```

**Thread Code for a client**

```
void* server_handle_client(void* arg)
{
    int* sock_client = ((int*)arg);
    if (sock_client == NULL)
    {
        return NULL;
    }

    int rc;
    char request_buffer[MESSAGE_SIZE];
    char response_buffer[MESSAGE_SIZE];
    while (true)
    {
        memset(request_buffer, 0, MESSAGE_SIZE);
        memset(response_buffer, 0, MESSAGE_SIZE);

        int received_bytes = recv(*sock_client, request_buffer, MESSAGE_SIZE, 0);
        
        rc = send(*sock_client, response_buffer, MESSAGE_SIZE, 0);

        if (strcmp(request_buffer, "exit") == 0
        || strcmp(request_buffer, "quit") == 0
        || strcmp(request_buffer, "shutdown") == 0)
        {
            break;
        }
    }

    if (sock_client != NULL)
    {
        free(sock_client);
    }

    return NULL;
}
```

## Simple UDP-Based Client-Server

Overall, the setup for UDP-Based client server application is similar with TCP-Based. I will show the different codes only.

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/10_peer_to_peer_client_server.c) for a complete source code.

![UDP-Based Client-Server](https://raw.githubusercontent.com/nguyenchiemminhvu/LinuxNetworkProgramming/refs/heads/main/udp_based_client_server.png)

### Necessary Headers And Macros

```
#define PROTOCOL "udp"
#define UDP_PORT 45123
#define MESSAGE_SIZE 1024
#define HOST_NAME "localhost"
```

Similar as previous explanation, but now the protocol is **UDP**.

### Setup Server

**Resolve Server Address**

```
struct protoent* udp_protocol = getprotobyname(PROTOCOL);

struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_DGRAM;
hints.ai_protocol = udp_protocol->p_proto;
struct addrinfo* addr_server;
rc = getaddrinfo(NULL, port_server, &hints, &addr_server); // INADDR_ANY
```

Specifies the socket type datagram for UDP connection.

**Server Loop - Listen client request and response**

```
while (1)
{
    struct sockaddr addr_client;
    socklen_t addr_client_len = sizeof(struct sockaddr);
    int received_bytes = recvfrom(sock_server, request_buffer, MESSAGE_SIZE, 0, &addr_client, &addr_client_len);

    sprintf(response_buffer, "Server received request at %d", time(NULL));
    int response_buffer_len = strlen(response_buffer);
    rc = sendto(sock_server, response_buffer, response_buffer_len, 0, &addr_client, addr_client_len);
}
```

The ```recvfrom()``` and ```sendto()``` functions are the general format of ```recv()``` and ```send()``` functions, they are suitable to use in UDP packet transferring.

### Setup Client

```
struct protoent* udp_protocol = getprotobyname(PROTOCOL);

struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_DGRAM;
hints.ai_protocol = udp_protocol->p_proto;
struct addrinfo* addr_server;
rc = getaddrinfo(HOST_NAME, port_server, &hints, &addr_server);
```

Specifies the socket type datagram for UDP connection.

**Client Loop - Send request and wait for response**

```
char request_buffer[MESSAGE_SIZE];
char response_buffer[MESSAGE_SIZE];
while (1)
{
    printf("Enter command: ");
    fgets(request_buffer, MESSAGE_SIZE, stdin);
    request_buffer[strcspn(request_buffer, "\r\n")] = '\0';

    int request_buffer_len = strlen(request_buffer);
    rc = sendto(sock_client, request_buffer, request_buffer_len, 0, addr_server->ai_addr, addr_server->ai_addrlen);

    int received_bytes = recvfrom(sock_client, response_buffer, MESSAGE_SIZE, 0, addr_server->ai_addr, &addr_server->ai_addrlen);
}
```

## Advanced Techniques

### Non-blocking sockets

Non-blocking sockets support to build responsive applications or handle multiple connections without blocking the main thread.

The code [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/11_non_blocking_sockets.c) demonstrates the use of non-blocking sockets in a simple TCP-based application.

In this example, the ```fcntl()``` function is used to set the server and client sockets to non-blocking mode.

```
#include <fcntl.h>

int fcntl(int fd, int cmd, ... /* arg */);
```

**Parameters:**

```fd```: The file descriptor to operate on. It must already be open.

```cmd```: The command to perform on the file descriptor. Common commands include:

- ```F_DUPFD```: Duplicate a file descriptor.
- ```F_GETFD```: Get the file descriptor flags.
- ```F_SETFD```: Set the file descriptor flags.
- ```F_GETFL```: Get the file status flags.
- ```F_SETFL```: Set the file status flags.
- ```F_SETLK```, ```F_SETLKW```, ```F_GETLK```: Manage file locks.

```arg (optional)```: An argument whose type and meaning depend on the cmd. It's typically an integer or a pointer, depending on the command.

**Utility Functions**

```
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
```

The utility function ```set_non_blocking()``` is used to configure the file descriptor of client and server sockets to operate non-blocking mode.

**Server socket initialization and binding**

```
struct protoent* tcp_proto = getprotobyname(PROTOCOL);
int sock_server = socket(addr_server->ai_family, addr_server->ai_socktype, addr_server->ai_protocol);
set_non_blocking(sock_server);
```

Creates a TCP socket and sets it to non-blocking mode.

Using ```set_non_blocking()``` ensures that the server does not block while waiting for connections.

**Accept client connection**

```
sock_client = accept(sock_server, &addr_client, &addr_client_len);
if (sock_client < 0)
{
    if (errno != EAGAIN && errno != EWOULDBLOCK)
    {
        report_error("Server accept() failed");
        break;
    }
    else
    {
        printf("No client connection\n");
    }
}
else
{
    set_non_blocking(sock_client);
}
```

Because server socket is non-blocking, the ```accept()``` call returns immediately. If there is no connection available, errno is set to ```EAGAIN``` or ```EWOULDBLOCK```.

**Receiving client data**

```
int received_bytes = recv(sock_client, buffer, MESSAGE_SIZE, 0);
if (received_bytes < 0)
{
    if (errno != EAGAIN && errno != EWOULDBLOCK)
    {
        report_error("Server recv() failed");
        break;
    }
}
```

Socket of client connection is set to non-blocking mode as well, ```recv()``` does not block when no data is available. If no data is received, errno is set to ```EAGAIN``` or ```EWOULDBLOCK```.

### Synchronous I/O Multiplexing with select()

Applying non-blocking file descriptor technique to network sockets allows the server to accept multiple client connection at a time without the need of using multithreading.

However, there is a limitation in the [previous sample code](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/11_non_blocking_sockets.c).

The Server Loop continuously checks for connections and data, which can lead to high CPU usage.

To address this, we can use I/O Multiplexing mechanism with the help of ```select()``` function.

```
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

**Parameters:**

```nfds```: Specifies the range of file descriptors to be checked. This is usually set to the highest file descriptor + 1.

```readfds```: A pointer to a set of file descriptors to monitor for readability. Use FD_SET() to add descriptors and FD_ISSET() to check them.

```writefds```: A pointer to a set of file descriptors to monitor for writability.

```exceptfds```: A pointer to a set of file descriptors to monitor for exceptional conditions.

```timeout```: A pointer to a struct timeval that specifies the maximum time to wait. It can be: ```NULL```: Wait indefinitely. ```Zero timeout```: Non-blocking mode, checks the status immediately. ```Specific value```: Blocks for the specified duration.

**Return Value:**

```> 0```: Number of file descriptors ready for I/O.

```0```: Timeout occurred, no file descriptors are ready.

```-1```: An error occurred, and errno is set appropriately.

The ```select()``` function in C is used for monitoring multiple file descriptors to see if they are ready for I/O operations such as reading, writing, or if there’s an exceptional condition. It’s commonly used in network programming for managing multiple sockets without multithreading.

```
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

```nfds```: The highest-numbered file descriptor + 1.

```readfds```: Set of FDs to check for readability.

```writefds```: Set of FDs to check for writability.

```exceptfds```: Set of FDs to check for exceptional conditions.

```timeout```: Maximum time ```select()``` should block, or ```NULL``` for indefinite blocking.

Checkout the completed sample code using I/O Multiplexing with ```select()``` function [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/12_multiplexing_select_client_server.c).

**Initialization of fd_set**

```
fd_set read_set;
fd_set master_set;
FD_ZERO(&master_set);
FD_SET(sock_server, &master_set);
global_max_fd = MAX(global_max_fd, sock_server);
```

```master_set``` keeps track of all file descriptors to monitor.

```read_set``` is a temporary copy used by select() to determine which descriptors are ready for I/O.

```global_max_fd``` variable is updated to the highest descriptor value for use in ```select()```.

**I/O Multiplexing with select()**

```
read_set = master_set;

int activity = select(global_max_fd + 1, &read_set, NULL, NULL, NULL);
if (activity < 0)
{
    report_error("Server select() failed");
}
```

```select()``` monitors the file descriptors in read_set for readability. It blocks until at least one descriptor is ready for reading.

**Handling Ready Descriptors**

```
for (int i = 0; i <= global_max_fd; i++)
{
    if (FD_ISSET(i, &read_set))
    {
        if (i == sock_server)
        {
            // Handle new incoming connections
        }
        else
        {
            // Handle client I/O
        }
    }
}
```

```FD_ISSET(i, &read_set)``` checks if descriptor i is ready for reading.

If I/O is ready on the server socket, it has a new connection to accept. Otherwise, the descriptor corresponds to a client socket, and data can be read from it.

**Accepting Server I/O**

```
struct sockaddr addr_client;
socklen_t addr_client_len = sizeof(struct sockaddr);
int sock_client = accept(sock_server, &addr_client, &addr_client_len);
FD_SET(sock_client, &master_set);
global_max_fd = MAX(global_max_fd, sock_client);
```

Adds the new client socket (sock_client) to ```master_set``` for monitoring.

Updates ```global_max_fd``` if the new socket's value is higher.

**Handling Client I/O**

```
int received_bytes = recv(i, request_buffer, MESSAGE_SIZE, 0);
if (received_bytes <= 0)
{
    // Client disconnected or error occurred
    close(i);
    FD_CLR(i, &master_set);
}
else
{
    // Process received data and send a response
    send(i, response_buffer, strlen(response_buffer), 0);
}
```

If received_bytes <= 0, the client either disconnected or an error occurred, then removes the socket from ```master_set```.

### Synchronous I/O Multiplexing with poll()

> WARNING: select() can monitor only file descriptors numbers that are less than FD_SETSIZE (1024)—an unreasonably low limit for many modern applications—and this limitation will not change. All modern applications should instead use poll(2) or epoll(7), which do not suffer this limitation.

Similar to ```select()```, the ```poll()``` function provides a way to monitor multiple file descriptors for readiness to perform I/O operations. However, ```poll()``` overcomes some limitations of ```select()```, such as the fixed size of the file descriptor set.

With ```poll()```, the server can efficiently handle multiple connections without needing multithreading, while addressing high CPU usage in the server loop.

```
#include <poll.h>
#include <unistd.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

**Parameters:**

```fds```: A pointer to an array of struct pollfd, which represents the file descriptors to monitor.

```nfds```: The number of file descriptors in the fds array.

```timeout```: Specifies the maximum time to wait (in milliseconds). It can be: ```-1```: Wait indefinitely. ```0```: Return immediately (non-blocking mode). ```Positive value```: Block for the specified time.

**Return Value:**

```> 0```: The number of file descriptors with events.

```0```: Timeout occurred, no events detected.

```-1```: An error occurred, and errno is set appropriately.

The ```poll()``` function is more scalable than ```select()``` for monitoring a large number of file descriptors. It is commonly used in network programming to manage multiple connections, enabling efficient I/O multiplexing.

Check out the complete code for poll() I/O multiplexing [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/13_multiplexing_poll_client_server.c).

**Initialization of pollfd array**

```
struct pollfd fds[MAX_CONNECTION];
memset(&fds, 0, sizeof(fds));
fds[0].fd = sock_server;    // Monitor server socket
fds[0].events = POLLIN;     // Monitor for incoming connections
int nfds = 1;               // Start with one monitored socket
```

The server socket is the first entry in the ```pollfd``` array, which is dynamically updated as clients connect or disconnect.

**Server Loop with poll()**

```
int activity = poll(fds, nfds, -1);  // Wait indefinitely
if (activity < 0)
{
    report_error("Server poll() failed");
    break;
}
```

The loop continuously monitors the file descriptors and handles events as they occur.

**Handle Server socket ready to read event**

```
if (fds[0].revents & POLLIN)
{
    struct sockaddr addr_client;
    socklen_t addr_client_len = sizeof(struct sockaddr);
    int sock_client = accept(fds[0].fd, &addr_client, &addr_client_len);
    if (nfds < MAX_CONNECTION)
    {
        fds[nfds].fd = sock_client;
        fds[nfds].events = POLLIN;  // Monitor for incoming data
        nfds++;
    }
}
```

Each new connection is added to the ```pollfd``` array, and the total monitored descriptors ```nfds``` is incremented.

**Handle client I/O**

```
for (int i = 1; i >= 1 && i < nfds; i++)
{
    if (fds[i].revents & POLLIN)
    {
        int received_bytes = recv(fds[i].fd, request_buffer, MESSAGE_SIZE, 0);
        if (received_bytes <= 0)
        {
            close(fds[i].fd);
            fds[i].fd = fds[nfds - 1].fd;  // Replace with the last descriptor
            nfds--;                        // Reduce the total count
            i--;
        }
        else
        {
            sprintf(response_buffer, "Server time: %ld", time(NULL));
            send(fds[i].fd, response_buffer, strlen(response_buffer), 0);
        }
    }
}
```

Receives data from the client.

Sends a response or disconnects if necessary.

Cleans up the ```pollfd``` array after disconnections by replacing the closed descriptor with the last one and reducing the monitored count.

### Broadcasting Messages

Broadcasting is a method in networking where a message is sent from one computer (called the sender) to all computers (called receivers) within the same network. This is like one person shouting a message in a room so that everyone in the room hears it (including yourself). The most common address for broadcasting is ```255.255.255.255```.

The full source code that demonstrate broadcasting socket can be found [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/14_broadcasting.c).

**Setup broadcast receiver socket**

```
int setup_broadcast_receiver(struct broadcast_t* receiver_info)
{
    int rc;

    receiver_info->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (receiver_info->fd < 0)
    {
        report_error("socket() failed for receiver");
        return -1;
    }

    int optval = 1;
    rc = setsockopt(receiver_info->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (rc != 0)
    {
        report_error("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    receiver_info->addr_receiver.sin_family = AF_INET;
    receiver_info->addr_receiver.sin_port = htons(BROADCAST_PORT);
    receiver_info->addr_receiver.sin_addr.s_addr = htonl(INADDR_ANY);
    receiver_info->addr_receiver_len = sizeof(receiver_info->addr_receiver);

    rc = bind(receiver_info->fd, (struct sockaddr *)&receiver_info->addr_receiver, receiver_info->addr_receiver_len);
    if (rc < 0)
    {
        report_error("bind() failed for receiver");
        return -1;
    }

    return 0;
}
```

```socket()```: Creates a UDP socket (```SOCK_DGRAM```) for communication.

```setsockopt()```: Configures the socket with ```SO_REUSEADDR``` option to allow binding the socket to an address that is already in use.

```bind()```: Binds the socket to a specific port (```BROADCAST_PORT```) on the local machine. It listens for messages sent to this port.

This receiver is set up to receive broadcast messages sent to the ```BROADCAST_PORT``` (defined as ```5555```).

**Setup broadcast sender socket**

```
int setup_broadcast_sender(struct broadcast_t* sender_info)
{
    int rc;

    sender_info->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender_info->fd < 0)
    {
        report_error("socket() failed for sender");
        return -1;
    }

    int optval = 1;
    rc = setsockopt(sender_info->fd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
    if (rc != 0)
    {
        report_error("setsockopt(SO_BROADCAST) failed");
        return -1;
    }

    sender_info->addr_receiver.sin_family = AF_INET;
    sender_info->addr_receiver.sin_port = htons(BROADCAST_PORT);
    inet_pton(AF_INET, BROADCAST_ADDR, &sender_info->addr_receiver.sin_addr);
    sender_info->addr_receiver_len = sizeof(sender_info->addr_receiver);

    return 0;
}
```

```socket()```: Creates a UDP socket (```SOCK_DGRAM```) for broadcasting.

```setsockopt()```: Configures the socket to allow broadcasting with the ```SO_BROADCAST``` option.

```inet_pton()```: Converts the broadcast IP address (```255.255.255.255```) from text to binary format to be used in the socket.

The sender is set to send broadcast messages to the specified address and port.

**Run receiver thread**

```
void* broadcast_receiver_thread_func(void* arg)
{
    struct broadcast_t* broadcast_receiver_info = (struct broadcast_t*)calloc(1, sizeof(struct broadcast_t));
    if (setup_broadcast_receiver(broadcast_receiver_info) != 0)
    {
        report_error("setup_broadcast_receiver() failed");
        return NULL;
    }

    char buffer[MESSAGE_SIZE];

    printf("Start to listen broadcast messages\n");
    while (1)
    {
        memset(buffer, 0, MESSAGE_SIZE);
        int received_bytes = recvfrom(broadcast_receiver_info->fd, buffer, MESSAGE_SIZE, 0, (struct sockaddr*)&broadcast_receiver_info->addr_receiver, &broadcast_receiver_info->addr_receiver_len);
        if (received_bytes <= 0)
        {
            report_error("Broadcast receiver recvfrom() failed");
        }
        else
        {
            printf("Received broadcast message: %s\n", buffer);
        }
    }

    close(broadcast_receiver_info->fd);
    free(broadcast_receiver_info);

    return NULL;
}
```

The function ```broadcast_receiver_thread_func``` runs in a separate thread.

It first calls ```setup_broadcast_receiver()``` to set up the receiver socket.

Then, it listens for incoming messages using ```recvfrom()```. Each received message is printed to the console.

The ```recvfrom()``` function reads the broadcast message into the buffer and prints it. If there is an error or no data is received, it reports the issue.

The receiver thread will continue to listen until the program is terminated.

**Run sender thread**

```
void* broadcast_sender_thread_func(void* arg)
{
    char* nick_name = (char*)arg;

    struct broadcast_t* broadcast_sender_info = (struct broadcast_t*)calloc(1, sizeof(struct broadcast_t));
    if (setup_broadcast_sender(broadcast_sender_info) != 0)
    {
        report_error("setup_broadcast_sender() failed");
        return NULL;
    }

    char broadcast_message[MESSAGE_SIZE];
    while (1)
    {
        memset(broadcast_message, 0, MESSAGE_SIZE);
        sprintf(broadcast_message, "%s is active", nick_name);
        int sent_bytes = sendto(broadcast_sender_info->fd, broadcast_message, MESSAGE_SIZE, 0, (struct sockaddr*)&broadcast_sender_info->addr_receiver, broadcast_sender_info->addr_receiver_len);
        if (sent_bytes <= 0)
        {
            report_error("Send broadcast message failed");
        }
        sleep(1);
    }

    close(broadcast_sender_info->fd);
    free(broadcast_sender_info);

    return NULL;
}
```

The function ```broadcast_sender_thread_func``` is responsible for sending broadcast messages to the broadcast address (```255.255.255.255```).

It sets up the sender socket by calling ```setup_broadcast_sender()```.

Inside a loop, it creates a message string containing the user's nickname and sends it via the ```sendto()``` function to the broadcast address every second.

## Create A Simple HTTP Server

![HTTP Server class diagram](https://raw.githubusercontent.com/nguyenchiemminhvu/LinuxNetworkProgramming/main/01_networking_libraries/my_http_server/http_server_design.png)

Full source code of my simple HTTP server is found [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/tree/main/01_networking_libraries/my_http_server).

This is a simple ```HTTP``` server written in ```C++``` using Linux socket programming. The server is designed to handle basic ```HTTP``` requests and responses. It listens for incoming connections, processes the requests, and sends back an appropriate response.

### Demo

**Server side:**

```
ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/my_http_server/build$ cmake ..

ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/my_http_server/build$ make

ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/my_http_server/build$ ./HTTPServer 8080

[1734346074] [INFO] 127.0.0.1:8080
[1734346074] [INFO] Server starts new poll()
[1734346086] [INFO] A client is connected
[1734346086] [INFO] 127.0.0.1:48146
[1734346086] [INFO] A client is disconnected
[1734346086] [INFO] Server starts new poll()
```

**Client side:**

```
ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/my_http_server/build$ curl -I http://localhost:8080

HTTP/1.1 200 OK
Content-Length:1544
Content-Length: 1544
```

```
ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/my_http_server/build$ curl http://localhost:8080

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Main Page</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            color: #333;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }
        h1 {
            font-size: 2.5em;
            color: #333;
            margin-bottom: 20px;
        }
        ul {
            list-style-type: none;
            padding: 0;
        }
        li {
            margin: 10px 0;
        }
        a {
            text-decoration: none;
            font-size: 1.2em;
            color: #007bff;
            padding: 10px 15px;
            border: 1px solid #007bff;
            border-radius: 5px;
            transition: background-color 0.3s, color 0.3s;
        }
        a:hover {
            background-color: #007bff;
            color: white;
        }
    </style>
</head>
<body>
    <h1>Main Page</h1>
    <p>Select a folder to view its contents:</p>
    <ul>
        <li><a href="./200">200 - OK</a></li>
        <li><a href="./400">400 - Bad Request</a></li>
        <li><a href="./403">403 - Forbidden</a></li>
        <li><a href="./404">404 - Not Found</a></li>
        <li><a href="./500">500 - Internal Server Error</a></li>
    </ul>
</body>
</html>
```

# Networking Libraries

## Using libcurl

[libcurl](https://curl.se/libcurl/) is a widely-used and powerful C library designed for transferring data over networks using a wide variety of protocols. It is the library behind the popular ```curl``` command-line tool and provides developers with a programmatic way to send and receive data through ```HTTP```, ```HTTPS```, ```FTP```, and other protocols.

Using libcurl is ideal for tasks that involve fetching web pages, uploading files to servers, interacting with ```REST APIs```, or sending emails... It saves time and effort because it eliminates the need to deal with low-level socket programming and protocol parsing. Instead of manually implementing low-level socket operations and parsing protocols, we can rely on libcurl to do the heavy lifting (creating network connections, handling requests, and managing data streams...).

### curl command-line examples

Fetches the content of http://example.com and saves it into a file called ```temp.txt```.

```
curl http://example.com > temp.txt
```

----

Downloads the content of http://example.com and saves it as ```index.html```.

```
curl http://example.com -o index.html
```

----

Downloads a file called ```file.zip``` from http://example.com

```
curl -O http://example.com/file.zip
```

----

Sends a ```POST``` request to http://example.com with data "name=ncmv".

```
curl -X POST -d "name=ncmv" http://example.com
```

----

Sends a ```POST``` request to http://example.com with ```JSON``` data ({"name":"John","age":30}).

```
curl -X POST -H "Content-Type: application/json" -d '{"name":"John","age":30}' http://example.com
```

----

Fetches only the headers of the ```HTTP``` response from http://example.com

```
curl -I http://example.com
```

----

Accesses http://example.com using ```HTTP``` Basic Authentication with the username ```username``` and password ```password```.

```
curl -u username:password http://example.com
```

----

Downloads the file ```readme.txt``` from the ```FTP``` server ```test.rebex.net``` using the username ```demo``` and password ```password```.

```
curl ftp://test.rebex.net/readme.txt --user demo:password
```

----

Uploads the local file temp to the ```FTP``` server ```test.rebex.net``` using the username ```demo``` and password ```password```.

```
curl -T temp ftp://test.rebex.net/ --user demo:password
```

----

Uploads the local file temp to the ```SFTP``` server at ```localhost``` into the folder ```/home/ncmv/study_workspace/``` using the username ```demo``` and password ```password```.

```
curl -u demo:passowrd -T temp sftp://localhost/home/ncmv/study_workspace/
```

----

Downloads the file temp from the ```SFTP``` server ```localhost``` (in the folder ```/home/ncmv/study_workspace/```) using the username ```demo``` and password ```password```, and saves it locally as ````temp````.

```
curl -u demo:password sftp://localhost/home/ncmv/study_workspace/temp -O temp
```

### Basic Curl

**Include necessary headers**

```
#include <iostream>
#include <curl/curl.h>
```

**Callback function for receiving HTTP response**

```
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    ((std::string*)userp)->append((char*)contents, total_size);
    return total_size;
}
```

Purpose: This function handles data received from the server during the HTTP request.

Parameters:

- ```contents```: A pointer to the data received.
- ```size``` and ```nmemb```: Together, they specify the size of the received data (in bytes).
- ```userp```: A user-provided pointer to store the received data (in this case, a std::string).

What it does:

- Calculates the total size of the data: size * nmemb.
- Appends the received data (converted to a string) to the ```std::string``` object passed in ```userp```.
- Returns the total size of the data to let libcurl know how much data was processed.

**Check libcurl version**

```
curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
if (info)
{
    std::cout << "libcurl version: " << info->version << std::endl;
    std::cout << "SSL version: " << info->ssl_version << std::endl;
    std::cout << "Libz version: " << info->libz_version << std::endl;
    std::cout << "Features: " << info->features << std::endl;

    const char *const *protocols = info->protocols;
    if (protocols)
    {
        std::cout << "Supported protocols: ";
        for (int i = 0; protocols[i] != NULL; ++i)
        {
            std::cout << protocols[i] << " ";
        }
        std::cout << std::endl;
    }
}
```

Purpose: Displays the version information and features supported by ```libcurl```.

How it works:

- Calls ```curl_version_info(CURLVERSION_NOW)``` to get information about the current version of ```libcurl```.
- Prints the version, ```SSL``` support, compression library (```Libz```), and the supported protocols (```HTTP```, ```HTTPS```, ```FTP```, ...).

**Initialize libcurl**

```
CURL *curl;
CURLcode res;
std::string readBuffer;

curl = curl_easy_init();
```

Details:

- ```CURL *curl```: A handle to manage the HTTP session.
- ```curl_easy_init()```: Initializes the handle. If successful, curl will not be NULL.

**Set libcurl options**

```
curl_easy_setopt(curl, CURLOPT_URL, "http://httpstat.us/200");
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
```

Purpose: Configures options for the HTTP request.

Options:

- ```CURLOPT_URL```: Sets the URL to request.
- ```CURLOPT_WRITEFUNCTION```: Specifies the callback function (```WriteCallback```) to handle the response data.
- ```CURLOPT_WRITEDATA```: Provides the ```std::string``` object (```readBuffer```) where the response data will be stored.

**Perform HTTP request**

```
res = curl_easy_perform(curl);
```

```curl_easy_perform(curl)```: Executes the HTTP request with the options set earlier.

**Clean up**

```
curl_easy_cleanup(curl);
```

Frees resources used by ```curl```. Always call this after finishing with ```curl```.

**Result**:

```
ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/libcurl/build$ ./basic_curl 

libcurl version: 8.5.0
SSL version: OpenSSL/3.0.13
Libz version: 1.3
Features: 1438599069
Supported protocols: dict file ftp ftps gopher gophers http https imap imaps ldap ldaps mqtt pop3 pop3s rtmp rtmpe rtmps rtmpt rtmpte rtmpts rtsp scp sftp smb smbs smtp smtps telnet tftp 

Response data: 200 OK
```

Full source code of basic curl example [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/01_networking_libraries/libcurl/src/basic_curl.cpp).

### Curl Multiple Handles

**Include necessary headers**

```
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>
```

**Define a Easy Handle struct**

```
struct CurlEasyHandle
{
    CURL* easy_handle;
    std::string url;
    std::string data;
};
```

Purpose: Stores information for each HTTP request.

- ```CURL* easy_handle```: A handle for making a single HTTP request.
- ```std::string url```: The URL to fetch.
- ```std::string data```: Stores the HTTP response data.

**Callback function for receiving HTTP response**

```
std::size_t perform_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
{
    std::string* str = static_cast<std::string*>(userdata);
    std::size_t total_size = size * nmemb;
    str->append(ptr, total_size);
    return total_size;
}
```

Purpose: Handles the data received from the server.

How it works:

- Calculates the total size of the received data: ```size * nmemb```.
- Appends this data to the ```std::string``` object pointed to by ```userdata```.
- Returns the total size of processed data to let ```libcurl``` know the data was handled.

**Callback function for downloading progress**

```
int perform_progress(void* ptr, double download_size, double downloaded, double upload_size, double uploaded)
{
    CurlEasyHandle* progData = (CurlEasyHandle*)ptr;
    std::cout << "Downloaded " << progData->url << ": " << downloaded << " bytes" << std::endl;

    return 0;
}
```

Purpose: Tracks the download progress for each URL.

How it works:

- Prints the number of bytes downloaded for the URL.
- Returning ```0``` signals ```libcurl``` to continue the download.
- Returning non-zero would stop it.

**Define a list of URLs**

```
const std::vector<std::string> urls = {
    "http://www.example.com",
    "http://www.google.com",
    "http://www.bing.com",
    "http://www.speedtest.net",
};
```

**Initialize libcurl**

```
CURLM* curl_multi;
int running_status;

curl_global_init(CURL_GLOBAL_DEFAULT);
curl_multi = curl_multi_init();
```

Purpose: Prepares libcurl for multi-handle operations.

Details:

- ```curl_global_init()```: Initializes global resources for libcurl.
- ```curl_multi_init()```: Creates a multi-handle for managing multiple simultaneous HTTP requests.

**Create Easy Handles and add to Multi Handle**

```
std::vector<CurlEasyHandle> easy_handles(urls.size());
for (int i = 0; i < urls.size(); i++)
{
    easy_handles[i].easy_handle = curl_easy_init();
    easy_handles[i].url = urls[i];

    curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_URL, urls[i].c_str());
    curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_WRITEFUNCTION, perform_callback);
    curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_WRITEDATA, &easy_handles[i].data);
    curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_PROGRESSFUNCTION, perform_progress);
    curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_PROGRESSDATA, &easy_handles[i]);

    curl_multi_add_handle(curl_multi, easy_handles[i].easy_handle);
}
```

Purpose: Creates and configures an easy handle for each URL, then adds it to the multi-handle.

Steps:

- Initialize a new easy handle using ```curl_easy_init()```.
- Configure each handle with:
  - The URL to fetch (```CURLOPT_URL```).
  - A callback for handling response data (```CURLOPT_WRITEFUNCTION```).
  - A pointer to the data storage (```CURLOPT_WRITEDATA```).
  - Progress monitoring options (```CURLOPT_NOPROGRESS```, ```CURLOPT_PROGRESSFUNCTION```, ```CURLOPT_PROGRESSDATA```).
- Add the easy handle to the multi-handle with ```curl_multi_add_handle()```.

**Perform Multi Handle request**

```
curl_multi_perform(curl_multi, &running_status);

do
{
    int curl_multi_fds;
    CURLMcode rc = curl_multi_perform(curl_multi, &running_status);
    if (rc == CURLM_OK)
    {
        rc = curl_multi_wait(curl_multi, nullptr, 0, 1000, &curl_multi_fds);
    }

    if (rc != CURLM_OK)
    {
        std::cerr << "curl_multi failed, code " << rc << std::endl;
        break;
    }
} while (running_status);
```

Purpose: Executes all HTTP requests simultaneously.

How it works:

- Starts the ```HTTP``` requests with ```curl_multi_perform()```.
- Continuously calls ```curl_multi_perform()``` in a loop until all requests are complete (running_status becomes 0).
- Uses ```curl_multi_wait()``` to wait for events (data availability) to avoid busy-waiting.

**Save data and clean up**

```
for (CurlEasyHandle& handle : easy_handles)
{
    std::string filename = handle.url.substr(11, handle.url.find_last_of(".") - handle.url.find_first_of(".") - 1) + ".html";
    std::ofstream file(filename);
    if (file.is_open())
    {
        file << handle.data;
        file.close();
        std::cout << "Data written to " << filename << std::endl;
    }

    curl_multi_remove_handle(curl_multi, handle.easy_handle);
    curl_easy_cleanup(handle.easy_handle);
}

curl_multi_cleanup(curl_multi);
curl_global_cleanup();
```

Purpose: Saves the response data to files, then cleans up resources.

Steps:

- For each handle:
  - Create a filename based on the ```URL```.
  - Save the response data to the file.
  - Remove the handle from the multi-handle (```curl_multi_remove_handle()```).
- Clean up the handle (```curl_easy_cleanup()```).
- Clean up the multi-handle (```curl_multi_cleanup()```) and global resources (```curl_global_cleanup()```).

**Result**:

```
ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/libcurl/build$ ./curl_multi_handle 

...
Downloaded http://www.speedtest.net: 167 bytes
...
Downloaded http://www.bing.com: 53057 bytes
...
Downloaded http://www.google.com: 57709 bytes
...
Downloaded http://www.example.com: 1256 bytes
...
Data written to example.html
Data written to google.html
Data written to bing.html
Data written to speedtest.html
```

Full source code of curl multiple handles example [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/01_networking_libraries/libcurl/src/curl_multi_handle.cpp).

### Curl Multithreading

**Include necessary headers**

```
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <curl/curl.h>
```

**Define structures**

```
struct ProgressData
{
    std::string url;
    double lastProgress;
};
```

Purpose: Stores progress information for each download.

Members:

- ```std::string url```: The URL being downloaded.
- ```double lastProgress```: The last recorded progress (in bytes) for this URL.

**Callback function for receiving HTTP response**

```
std::size_t perform_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
{
    std::string* str = static_cast<std::string*>(userdata);
    std::size_t total_size = size * nmemb;
    str->append(ptr, total_size);
    return total_size;
}
```

Purpose: Handles data received from the server during an ```HTTP``` request.

Details:

- Appends received data to a ```std::string``` provided as userdata.
- Returns the size of the data to confirm successful processing.

**Callback function for downloading progress**

```
int perform_progress(void* ptr, double download_size, double downloaded, double upload_size, double uploaded)
{
    ProgressData* progData = (ProgressData*)ptr;

    if (downloaded - progData->lastProgress >= 1024.0)
    {
        std::cout << "Download " << progData->url << ": " << downloaded << " bytes" << std::endl;
        progData->lastProgress = downloaded;
    }

    return 0;
}
```

Purpose: Tracks and displays download progress for a specific URL.

Details:

- Checks if at least 1 KB (1024 bytes) of new data has been downloaded since the last update.
- Prints the progress and updates lastProgress.

**Function to perform HTTP request**

```
void perform_request(const std::string& url)
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl != nullptr)
    {
        std::string data;
        ProgressData progData;
        progData.url = url;
        progData.lastProgress = 0.0;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, perform_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, perform_progress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progData);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            std::string filename = url.substr(11, url.find_last_of(".") - url.find_first_of(".") - 1) + ".html";
            std::ofstream file(filename);
            if (file.is_open())
            {
                file << data;
                file.close();
                std::cout << "Data written to " << filename << std::endl;
            }
        }
    }
}
```

Purpose: Performs an ```HTTP``` request to download the content of a given ```URL```.

Steps:

- Initializes a CURL easy handle.
- Sets up callbacks for data writing (```perform_callback```) and progress tracking (```perform_progress```).
- Executes the request using ```curl_easy_perform()```.

On success:

- Saves the downloaded data to a file named after the ```URL```.

**Set up multithreading HTTP perform**

```
curl_global_init(CURL_GLOBAL_ALL);

std::vector<std::thread> threads;
std::vector<std::string> urls = {
    "http://www.example.com",
    "http://www.google.com",
    "http://www.bing.com",
    "http://www.speedtest.net",
};

for (const std::string& url : urls)
{
    threads.push_back(std::thread(perform_request, url));
}

for (std::thread& t : threads)
{
    t.join();
}

curl_global_cleanup();
```

Details:

- ```curl_global_init(CURL_GLOBAL_ALL)```: Prepares libcurl for multi-threaded operations.
- Creates a vector to store thread objects and another to store the list of URLs.
- For each URL, creates a new thread to execute ```perform_request()```.
- Ensures all threads complete before the program exits using ```join()``` method.
- ```curl_global_cleanup()```: Releases resources allocated by ```libcurl```.

Full source code of basic curl multithreading [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/01_networking_libraries/libcurl/src/curl_multithreaded.cpp).

## Secure Networking with OpenSSL

```SSL``` (Secure Sockets Layer) is a cryptographic protocol originally designed to provide secure communication over a network, such as the internet. It ensures that the data transferred between a client (e.g., a web browser) and a server (e.g., a website) is encrypted, authenticated, and protected from being tampered with.

Modern systems use ```TLS``` (Transport Layer Security), which is an updated and more secure version of ```SSL```. When people say ```SSL```, they often mean ```SSL/TLS```.

One of well-known ```SSL/TLS``` application is HTTPs protocol. TLS encryption method is used to secure communication on the web, such as browsing, submit forms, online payments...

**SSL Handshake:**

- The client (e.g., a browser) connects to the server and says, "I want to use SSL/TLS."
- The server sends back its certificate, which contains its identity and a public key.
- The client verifies the server's certificate to ensure it’s legitimate.
- The client and server agree on a shared "session key" to encrypt the data during the session.

![How HTTPs work](https://raw.githubusercontent.com/nguyenchiemminhvu/LinuxNetworkProgramming/refs/heads/main/how_https_work.png)

To work with ```SSL/TLS``` protocol in programming, ```OpenSSL``` is a typical choice.

**Installation**

```
sudo apt-get install libssl-dev openssl
```

```libssl-dev```: Contains the development libraries for ```OpenSSL```, which are needed to compile programs using ```OpenSSL```.

```openssl```: Installs the ```OpenSSL``` command-line tool, which can be used for generating keys and certificates or debugging ```SSL/TLS``` issues.

**Initialize OpenSSL**

```
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_library_init();
SSL_load_error_strings();
OpenSSL_add_all_algorithms();
```

These steps ensure that ```OpenSSL``` is ready to handle cryptography and provide meaningful error messages in case something goes wrong.

**Create SSL context**

```
SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());  // For server

SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());  // For client
```

```TLS_server_method()```: Configures the context for use in server mode.

```TLS_client_method()```: Configures the context for use in client mode.

The ```SSL_CTX``` structure holds protocol settings, certificates, and other necessary configurations.

**Load Certificates (only for server)**

```
SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM);

SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM);
```

```server.crt```: The server's certificate file (proves the server's identity to the client).

```server.key```: The private key file associated with the certificate.

This step ensures that the server can provide authentication during the ```SSL/TLS ```handshake.

**Create and Bind socket**

Set up a regular ```TCP``` socket as you would for normal network programming.

**Wrap socket with SSL**

```
SSL *ssl = SSL_new(ctx);
SSL_set_fd(ssl, socket_fd);
```

The ```SSL``` object manages the encryption and decryption for the socket connection.

**Perform Handshake**

```
SSL_accept(ssl); // For server

SSL_connect(ssl); // For client
```

The ```SSL/TLS``` handshake authenticates the server (and optionally the client) and establishes an encrypted communication channel.

```SSL_accept()```: The server waits for the client to initiate the handshake.

```SSL_connect()```: The client initiates the handshake with the server.

**Send and receive encrypted data**

After the handshake, the ```SSL``` connection is ready to send and receive encrypted data.

```
SSL_write(ssl, "Hello, Secure World!", strlen("Hello, Secure World!"));
char buffer[1024];

SSL_read(ssl, buffer, sizeof(buffer));
```

**Cleanup**

```
SSL_shutdown(ssl);
SSL_free(ssl);
SSL_CTX_free(ctx);
```

With all these steps, It is enough to establish secure communication between a client and server using the ```SSL/TLS``` protocol with ```OpenSSL``` library.

### A HTTPS Client

Full source code of the example HTTPs Client is found [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/01_networking_libraries/openssl/src/https_client.c).

**Result**:

```
ncmv@localhost:~/study_workspace/LinuxNetworkProgramming/01_networking_libraries/openssl/build$ ./https_client example.com 443

93.184.215.14:443
SSL connection is done with cipher suite TLS_AES_256_GCM_SHA384

Received 361 bytes
Received 1256 bytes
HTTP/1.1 200 OK
Age: 140532
Cache-Control: max-age=604800
Content-Type: text/html; charset=UTF-8
Date: Sat, 14 Dec 2024 09:44:47 GMT
Etag: "3147526947+gzip+ident"
Expires: Sat, 21 Dec 2024 09:44:47 GMT
Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT
Server: ECAcc (sed/58B0)
Vary: Accept-Encoding
X-Cache: HIT
Content-Length: 1256
Connection: close

(The remaining is HTTP content of example.com website)
```

### Secure Client-Server

| SSL Server Workflow             | SSL Client Workflow             |
|----------------------------------|----------------------------------|
| ![SSL Server Workflow](https://raw.githubusercontent.com/nguyenchiemminhvu/LinuxNetworkProgramming/refs/heads/main/SSL_server_workflow.png) | ![SSL Client Workflow](https://raw.githubusercontent.com/nguyenchiemminhvu/LinuxNetworkProgramming/refs/heads/main/SSL_client_workflow.png) |

Full source code of the example SSL Client-Server is found [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/01_networking_libraries/openssl/src/ssl_client_server.c).

# Conclusion

**Reference**:

https://www.linuxhowtos.org/C_C++/socket.htm

https://www.tutorialspoint.com/unix_sockets/index.htm

https://documentation.softwareag.com/adabas/wcp632mfr/wtc/wtc_prot.htm

https://www.geeksforgeeks.org/little-and-big-endian-mystery/

https://github.com/openssl/openssl/tree/691064c47fd6a7d11189df00a0d1b94d8051cbe0/demos/ssl
