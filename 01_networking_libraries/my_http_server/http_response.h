#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <unordered_map>

class HTTPResponse
{
public:
    HTTPResponse(int code = 404, const std::string& body = "404 Not Found");
    void set_header(const std::string& key, const std::string& val);
    std::string to_string();

private:
    std::string code_to_message(int code);

private:
    int m_status;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
};

#endif // HTTP_RESPONSE_H