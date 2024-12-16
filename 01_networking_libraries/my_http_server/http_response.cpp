#include "http_response.h"

HTTPResponse::HTTPResponse(int code, const std::string& body)
{

}

void HTTPResponse::set_header(const std::string& key, const std::string& val)
{
    m_headers[key] = val;
}

std::string HTTPResponse::to_string()
{
    return "";
}

std::string HTTPResponse::code_to_message(int code)
{
    return "";
}