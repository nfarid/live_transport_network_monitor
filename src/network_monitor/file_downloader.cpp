
#include <network_monitor/file_downloader.hpp>

#include <curl/curl.h>

#include <cstdio>
#include <memory>
#include <fstream>
#include <iostream>


namespace NetworkMonitor {

bool downloadFile(
    const std::string& fileURL,
    const std::filesystem::path& destination,
    const std::filesystem::path& cacertFile
)
{
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl{
        curl_easy_init(),
        curl_easy_cleanup,
    };
    if(!curl) {
        std::cerr<<"Unable to initialise curl"<<std::endl;
        return false;
    }
    CURLcode err = CURLE_OK;

    err = curl_easy_setopt(curl.get(), CURLOPT_URL, fileURL.c_str() );
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_URL error: "<<curl_easy_strerror(err)<<std::endl;
        return false;
    }

    err = curl_easy_setopt(curl.get(), CURLOPT_CAINFO, cacertFile.c_str() );
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_CAINFO: "<<curl_easy_strerror(err)<<std::endl;
        return false;
    }

    err = curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_FOLLOWLOCATION error: "<<curl_easy_strerror(err)<<std::endl;
        return false;
    }

    std::unique_ptr<FILE, decltype(&std::fclose)> inputFile{
        std::fopen(destination.c_str(), "wb"),
        std::fclose,
    };
    if(!inputFile) {
        std::perror("fopen error: ");
        return false;
    }
    err = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, inputFile.get() );
    if(err != CURLE_OK) {
        std::cerr<<"CURLOPT_WRITEDATA error: "<<curl_easy_strerror(err)<<std::endl;
        return false;
    }

    err = curl_easy_perform(curl.get() );
    if(err != CURLE_OK) {
        std::cerr<<"CURL file transfer error: "<<curl_easy_strerror(err)<<std::endl;
        return false;
    }

    return true;
}

nlohmann::json parseJsonFile(const std::filesystem::path& source) {
    if(!std::filesystem::exists(source) ) {
        std::cerr<<__func__<<":"<<__LINE__<<": unable to find line: "<<source<<std::endl;
        return nlohmann::json::value_t::discarded;
    }
    std::ifstream inputFile{source};
    try {
        return nlohmann::json::parse(inputFile);
    } catch(std::exception& ex) {
        std::cerr<<__func__<<":"<<__LINE__<<": JSON parsing error: "<<ex.what()<<std::endl;
        return nlohmann::json::value_t::discarded;
    }
}

} //NetworkMonitor
