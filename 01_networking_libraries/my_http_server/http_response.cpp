#include "http_response.h"

#include <string>
#include <sstream>
#include <cstring>

HTTPResponse::HTTPResponse(int code, const std::string& body)
{

}

void HTTPResponse::set_status(int status)
{
    m_status = status;
}

void HTTPResponse::set_body(const std::string& body)
{
    m_body = body;
    set_header("Content-Length", std::to_string(m_body.length()));
}

void HTTPResponse::set_header(const std::string& key, const std::string& val)
{
    m_headers[key] = val;
}

std::string HTTPResponse::to_string()
{
    std::ostringstream oss;
    oss << "HTTP/1.1 " << m_status << " " << code_to_message(m_status) << "\r\n";
    for (const auto& header : m_headers)
    {
        oss << header.first << ":" << header.second << "\r\n";
    }
    oss << "Content-Length: " << m_body.length() << "\r\n";
    oss << "\r\n";
    oss << m_body;
    return oss.str();
}

std::string HTTPResponse::code_to_message(int code)
{
    switch (code)
    {
        case 200: return "OK";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}