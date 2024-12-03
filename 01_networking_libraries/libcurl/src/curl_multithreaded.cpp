#include <iostream>
#include <thread>
#include <vector>
#include <fstream>

#include <curl/curl.h>

using namespace std;

struct ProgressData
{
    std::string url;
    double lastProgress;
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
    ProgressData* progData = (ProgressData*)ptr;

    if (downloaded - progData->lastProgress >= 1024.0)
    {
        std::cout << "Download " << progData->url << ": " << downloaded << " bytes" << std::endl;
        progData->lastProgress = downloaded;
    }

    return 0;
}

void perform_request(const std::string& url)
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl != nullptr)
    {
        std::string data;
        ProgressData progData;
        progData.url = url;
        progData.lastProgress = 0.0;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, perform_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, perform_progress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progData);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            std::string filename = url.substr(11, url.find_last_of(".") - url.find_first_of(".") - 1) + ".html";
            std::ofstream file(filename);
            if (file.is_open())
            {
                file << data;
                file.close();
                std::cout << "Data written to " << filename << std::endl;
            }
        }
        else
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        
    }
}

int main()
{
    curl_global_init(CURL_GLOBAL_ALL);

    std::vector<std::thread> threads;
    std::vector<std::string> urls = {
        "http://www.example.com",
        "http://www.google.com",
        "http://www.bing.com",
        "http://www.speedtest.net",
    };

    for (const std::string& url : urls)
    {
        threads.push_back(std::thread(perform_request, url));
    }

    for (std::thread& t : threads)
    {
        t.join();
    }

    curl_global_cleanup();

    return 0;
}