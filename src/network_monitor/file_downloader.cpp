
#include <network_monitor/file_downloader.hpp>

#include <curl/curl.h>

#include <cstdio>
#include <iostream>


namespace NetworkMonitor {

bool downloadFile(
    const std::string& fileURL,
    const std::filesystem::path& destination,
    const std::filesystem::path& cacertFile
)
{
    CURL* curl = curl_easy_init();
    if(!curl) {
        std::cerr<<"Unable to initialise curl"<<std::endl;
        return false;
    }
    CURLcode err = CURLE_OK;

    err = curl_easy_setopt(curl, CURLOPT_URL, fileURL.c_str() );
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_URL error: "<<curl_easy_strerror(err)<<std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    err = curl_easy_setopt(curl, CURLOPT_CAINFO, cacertFile.c_str() );
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_CAINFO: "<<curl_easy_strerror(err)<<std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    err = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_FOLLOWLOCATION error: "<<curl_easy_strerror(err)<<std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    FILE* inputFile = std::fopen(destination.c_str(), "wb");
    if(!inputFile) {
        std::perror("fopen error: ");
        curl_easy_cleanup(curl);
        return false;
    }
    err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, inputFile);
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_WRITEDATA error: "<<curl_easy_strerror(err)<<std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    err = curl_easy_perform(curl);
    if(err != CURLE_OK) {
        std::cerr<<"CURL file transfer error: "<<curl_easy_strerror(err)<<std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_cleanup(curl);
    return true;
}

}
