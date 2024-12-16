#include "http_parser.h"

#include <string>
#include <cstring>
#include <sstream>

HTTPRequest HTTPParser::parse(const std::string& raw_request)
{
    HTTPRequest request;

    std::istringstream iss(raw_request);
    std::string line;

    // First line: method and path
    if (std::getline(iss, line) && line != "\r")
    {
        std::istringstream iss_first_line(line);
        iss_first_line >> request.m_method >> request.m_path;
    }

    while (std::getline(iss, line) && line != "\r")
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        std::size_t pivot = line.find(':');
        if (pivot != std::string::npos)
        {
            std::string key = line.substr(0, pivot);
            std::string val = line.substr(pivot + 1);

            if (!val.empty() && val[0] == ' ')
            {
                val.erase(0, 1);
            }
            request.m_headers[key] = val;
        }
    }

    if (request.m_headers.count("Content-Length") > 0)
    {
        std::size_t body_length = std::stoi(request.m_headers["Content-Length"]);
        request.m_body.resize(body_length);
        iss.read(&request.m_body[0], body_length);
    }

    return request;
}