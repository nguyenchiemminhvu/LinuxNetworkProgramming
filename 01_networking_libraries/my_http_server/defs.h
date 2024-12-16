#ifndef DEFS_H
#define DEFS_H

#define STR_LOCALHOST "localhost"
#define STR_LOCALHOST_IP "127.0.0.1"
#define STR_TCP_PROTOCOL "tcp"
#define MAX_CONNECTION (2)
#define MESSAGE_SIZE (1024)

enum ClientActivity
{
    UNKNOWN = -1,
    WAITING,
    DISCONNECT,
    COMPLETED,
};

#endif // DEFS_H