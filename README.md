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
- [Networking Libraries](#networking-libraries)
  - [Using libcurl](#using-libcurl)
    - [Basic Curl](#basic-curl)
    - [Curl Multiple Handle](#curl-multiple-handle)
    - [Curl Multithreading](#curl-multithreading)
  - [Secure Networking with OpenSSL](#secure-networking-with-openssl)
- [Conclusion](#conclusion)


# Introduction

Linux network programming deals with the interaction between processes using network interfaces. It enables interprocess communication (IPC), allowing data exchange between processes running on the same machine or on different machines connected over a network.

The foundation of Linux network programming lies in the use of sockets, a universal API designed for interprocess communication. Sockets originated from BSD Unix in 1983 and were later standardized by POSIX, making them a cornerstone of modern networking.

# Sockets

A socket is an endpoint for communication. Think of it as a door through which data flows in and out of a process. Processes use sockets to send and receive messages, enabling seamless IPC.

Sockets were initially designed to support two domains:

**Unix Domain (Unix)**: Used for communication between processes within the same operating system.

**Internet Domain (INET)**: Used for communication between processes on different systems connected via a TCP/IP network.

Unix domain sockets are used for IPC within the same operating system. They are faster than INET sockets because they don't require network protocol overhead. Instead of IP addresses, Unix domain sockets use file system paths for addressing.

INET domain sockets are used for communication between processes on different systems connected over a network. These sockets rely on the TCP/IP protocol stack, which ensures data integrity and delivery.

Two common protocols used with INET domain sockets are:

**TCP (Transmission Control Protocol)**: Provides reliable, ordered, and error-checked delivery of data.

**UDP (User Datagram Protocol)**: Provides fast, connectionless data transmission without guarantees of delivery.

## Type of sockets

The BSD socket API supports several types of sockets, which determine how data is transmitted between processes:

**Stream Sockets (SOCK_STREAM)**: These provide a reliable, connection-oriented communication protocol. Data is sent and received as a continuous stream of bytes. Typically used with TCP (Transmission Control Protocol).

**Datagram Sockets (SOCK_DGRAM)**: These provide a connectionless communication protocol. Data is sent in discrete packets, and delivery isn't guaranteed. Typically used with UDP (User Datagram Protocol).

**Raw Sockets (SOCK_RAW)**: These allow processes to access lower-level network protocols directly, bypassing the standard TCP or UDP layers. Useful for custom protocol implementations or network monitoring tools.

## Addressing sockets

In the INET domain, sockets are identified by two components:

**IP Address**: A 32-bit number (IPv4) or a 128-bit number (IPv6) that uniquely identifies a device on a network. IPv4 addresses are often represented in dotted decimal notation, such as 192.168.1.1.

**Port Number**: A 16-bit number that identifies a specific service or application on the device. For example, web servers typically use port 80 (HTTP) or 443 (HTTPS).

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

- we want to write a program which gets the address of a WWW site (e.g. httpstat.us) as the argument and fetches the document.
- the program outputs the document to stdout;
- the program uses TCP to connect to the HTTP server.

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/07_http_redirection.cpp) for a complete source code.

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
protoent* p_proto_ent = getprotobyname("tcp");
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
addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_protocol = p_proto_ent->p_proto;
hints.ai_socktype = SOCK_STREAM;

addrinfo* server_addr;
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
rc = connect(sock_fd, server_addr->ai_addr, sizeof(sockaddr));
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

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/08_simple_client_server.cpp) for a complete source code.

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
protoent* tcp_proto = getprotobyname(PROTOCOL);
```

Retrieves the protocol structure for the ```"tcp"``` protocol using ```getprotobyname()```.

```
char server_port[6];
memset(server_port, 0, 6);
sprintf(server_port, "%d", htons(TCP_PORT));

addrinfo addr_hints;
memset(&addr_hints, 0, sizeof(addr_hints));
addr_hints.ai_family = AF_INET;
addr_hints.ai_socktype = SOCK_STREAM;
addr_hints.ai_protocol = tcp_proto->p_proto;

addrinfo* addr_server;
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
sockaddr addr_client;
socklen_t addr_len = sizeof(addr_client);
sock_client = accept(sock_server, (sockaddr*)&addr_client, &addr_len);
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
protoent* tcp_proto = getprotobyname(PROTOCOL);

addrinfo addr_hints;
memset(&addr_hints, 0, sizeof(addr_hints));
addr_hints.ai_family = AF_INET;
addr_hints.ai_socktype = SOCK_STREAM;
addr_hints.ai_protocol = tcp_proto->p_proto;

addrinfo* addr_server;
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

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/09_multithread_client_server.cpp) for a complete source code.

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

Click [HERE](https://github.com/nguyenchiemminhvu/LinuxNetworkProgramming/blob/main/00_tutorials/10_peer_to_peer_client_server.cpp) for a complete source code.

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
protoent* udp_protocol = getprotobyname(PROTOCOL);

addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_DGRAM;
hints.ai_protocol = udp_protocol->p_proto;
addrinfo* addr_server;
rc = getaddrinfo(NULL, port_server, &hints, &addr_server); // INADDR_ANY
```

Specifies the socket type datagram for UDP connection.

**Server Loop - Listen client request and response**

```
while (true)
{
    sockaddr addr_client;
    socklen_t addr_client_len = sizeof(sockaddr);
    int received_bytes = recvfrom(sock_server, request_buffer, MESSAGE_SIZE, 0, &addr_client, &addr_client_len);

    sprintf(response_buffer, "Server received request at %d", time(NULL));
    int response_buffer_len = strlen(response_buffer);
    rc = sendto(sock_server, response_buffer, response_buffer_len, 0, &addr_client, addr_client_len);
}
```

The ```recvfrom()``` and ```sendto()``` functions are the general format of ```recv()``` and ```send()``` functions, they are suitable to use in UDP packet transferring.

### Setup Client

```
protoent* udp_protocol = getprotobyname(PROTOCOL);

addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_DGRAM;
hints.ai_protocol = udp_protocol->p_proto;
addrinfo* addr_server;
rc = getaddrinfo(HOST_NAME, port_server, &hints, &addr_server);
```

Specifies the socket type datagram for UDP connection.

**Client Loop - Send request and wait for response**

```
char request_buffer[MESSAGE_SIZE];
char response_buffer[MESSAGE_SIZE];
while (true)
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

```

```

### Synchronous I/O Multiplexing with select()

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

### Synchronous I/O Multiplexing with poll()

```

```

# Networking Libraries

## Using libcurl

### Basic Curl

### Curl Multiple Handle

### Curl Multithreading

## Secure Networking with OpenSSL

# Conclusion

**Reference**:

https://www.linuxhowtos.org/C_C++/socket.htm

https://www.tutorialspoint.com/unix_sockets/index.htm

https://documentation.softwareag.com/adabas/wcp632mfr/wtc/wtc_prot.htm

https://www.geeksforgeeks.org/little-and-big-endian-mystery/
