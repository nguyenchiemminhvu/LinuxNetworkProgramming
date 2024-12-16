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

    LOGD(path.c_str());

    std::string filename = path + "index.html";
    LOGD(filename);
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
            LOGD("File content read successfully.");
            LOGD(html_content);
            response.set_body(html_content);
        }
        else
        {
            std::cerr << "Error reading the file." << std::endl;
        }

        file.close();
    }

    return response;
}