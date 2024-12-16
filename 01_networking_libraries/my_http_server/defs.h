#ifndef DEFS_H
#define DEFS_H

#define STR_HTTP_ROOT_PATH "http_root"
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

enum HttpStatus
{
    HTTP_200 = 200,
    HTTP_400 = 400,
    HTTP_403 = 403,
    HTTP_404 = 404,
    HTTP_500 = 500
};

#endif // DEFS_H