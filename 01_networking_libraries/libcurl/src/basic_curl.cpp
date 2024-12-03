#include <iostream>

#include <curl/curl.h>

// Callback function to write data received from the server
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    ((std::string*)userp)->append((char*)contents, total_size);
    return total_size;
}

int main()
{
    curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
    if (info)
    {
        std::cout << "libcurl version: " << info->version << std::endl;
        std::cout << "SSL version: " << info->ssl_version << std::endl;
        std::cout << "Libz version: " << info->libz_version << std::endl;
        std::cout << "Features: " << info->features << std::endl;

        const char *const *protocols = info->protocols;
        if (protocols)
        {
            std::cout << "Supported protocols: ";
            for (int i = 0; protocols[i] != NULL; ++i)
            {
                std::cout << protocols[i] << " ";
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to get libcurl version info." << std::endl;
    }

    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    // Initialize libcurl
    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000");

        // Set the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            // Print the response data
            std::cout << "Response data: " << readBuffer << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "Failed to initialize libcurl" << std::endl;
    }

    return 0;
}