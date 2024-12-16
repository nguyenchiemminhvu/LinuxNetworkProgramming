#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "http_request.h"
#include <string>

class HTTPParser
{
public:
    static HTTPRequest parse(const std::string& raw_request);
};

#endif // HTTP_PARSER_H