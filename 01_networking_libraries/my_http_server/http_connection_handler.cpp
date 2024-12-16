#include "http_connection_handler.h"
#include "logging.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

HTTPConnectionHandler::HTTPConnectionHandler()
{
    
}

ClientActivity HTTPConnectionHandler::handle_client(int sock_client)
{
    char buffer[MESSAGE_SIZE];
    std::memset(buffer, 0, MESSAGE_SIZE);
    int rc_recv = recv(sock_client, buffer, MESSAGE_SIZE, 0);
    if (rc_recv <= 0)
    {
        LOGI("Receive zero data from client");
        return ClientActivity::DISCONNECT;
    }

    HTTPRequest client_request = m_parser.parse(buffer);
    HTTPResponse server_response = m_router.route(client_request);

    std::string s_server_response = server_response.to_string();
    int rc_send = send(sock_client, s_server_response.c_str(), s_server_response.length(), 0);
    if (rc_send <= 0)
    {
        LOGE("Server is failed to respond");
        return ClientActivity::DISCONNECT;
    }

    return ClientActivity::COMPLETED;
}