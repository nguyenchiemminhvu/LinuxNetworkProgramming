#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <curl/curl.h>

using namespace std;

struct CurlEasyHandle
{
    CURL* easy_handle;
    std::string url;
    std::string data;
};

std::size_t perform_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
{
    std::string* str = static_cast<std::string*>(userdata);
    std::size_t total_size = size * nmemb;
    str->append(ptr, total_size);
    return total_size;
}

int perform_progress(void* ptr, double download_size, double downloaded, double upoad_size, double uploaded)
{
    CurlEasyHandle* progData = (CurlEasyHandle*)ptr;
    std::cout << "Downloaded " << progData->url << ": " << downloaded << " bytes" << std::endl;

    return 0;
}

int main()
{
    const std::vector<std::string> urls = {
        "http://www.example.com",
        "http://www.google.com",
        "http://www.bing.com",
        "http://www.speedtest.net",
    };

    CURLM* curl_multi;
    int running_status;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl_multi = curl_multi_init();

    std::vector<CurlEasyHandle> easy_handles(urls.size());
    for (int i = 0; i < urls.size(); i++)
    {
        easy_handles[i].easy_handle = curl_easy_init();
        easy_handles[i].url = urls[i];

        curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_URL, urls[i].c_str());
        curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_WRITEFUNCTION, perform_callback);
        curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_WRITEDATA, &easy_handles[i].data);
        curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_PROGRESSFUNCTION, perform_progress);
        curl_easy_setopt(easy_handles[i].easy_handle, CURLOPT_PROGRESSDATA, &easy_handles[i]);

        curl_multi_add_handle(curl_multi, easy_handles[i].easy_handle);
    }

    curl_multi_perform(curl_multi, &running_status);

    do
    {
        int curl_multi_fds;
        CURLMcode rc = curl_multi_perform(curl_multi, &running_status);
        if (rc == CURLM_OK)
        {
            rc = curl_multi_wait(curl_multi, nullptr, 0, 1000, &curl_multi_fds);
        }

        if (rc != CURLM_OK)
        {
            std::cerr << "curl_multi failed, code " << rc << std::endl;
            break;
        }
    } while (running_status);
    
    for (CurlEasyHandle& handle : easy_handles)
    {
        std::string filename = handle.url.substr(11, handle.url.find_last_of(".") - handle.url.find_first_of(".") - 1) + ".html";
        std::ofstream file(filename);
        if (file.is_open())
        {
            file << handle.data;
            file.close();
            std::cout << "Data written to " << filename << std::endl;
        }

        curl_multi_remove_handle(curl_multi, handle.easy_handle);
        curl_easy_cleanup(handle.easy_handle);
    }

    curl_multi_cleanup(curl_multi);
    curl_global_cleanup();

    return 0;
}