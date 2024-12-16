#include "http_router.h"
#include "logging.h"
#include "defs.h"

#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <filesystem>

HTTPResponse HTTPRouter::route(const HTTPRequest& request)
{
    HTTPResponse response;

    std::string path = std::string(STR_HTTP_ROOT_PATH) + request.m_path;
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        LOGE("Path is not found");
        path = std::string(STR_HTTP_ROOT_PATH) + std::string("/404");
        response.set_status(HTTP_404);
    }
    else
    {
        response.set_status(HTTP_200);
    }


    std::string filename = path + STR_HTTP_MAIN_PAGE;
    std::fstream file(filename, std::ios::in);
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        std::streampos file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string html_content;
        html_content.resize(file_size);

        file.read(&html_content[0], file_size);

        if (file)
        {
            response.set_body(html_content);
        }
        else
        {
           LOGE("Error reading the file");
        }

        file.close();
    }

    return response;
}