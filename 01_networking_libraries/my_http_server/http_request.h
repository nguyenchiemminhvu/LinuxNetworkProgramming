#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>

class HTTPRequest
{
public:
    std::string m_method;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;

    HTTPRequest() = default;
};

#endif // HTTP_REQUEST_H