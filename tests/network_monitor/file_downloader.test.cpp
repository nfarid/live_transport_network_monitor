
#include <network_monitor/file_downloader.hpp>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using NetworkMonitor::downloadFile, NetworkMonitor::parseJsonFile;
namespace fs = std::filesystem;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(network_layout_file)
{
    BOOST_TEST(std::filesystem::exists(TEST_NETWORK_LAYOUT) );
}


BOOST_AUTO_TEST_CASE(file_downloader)
{
    const std::string fileUrl = "https://ltnm.learncppthroughprojects.com/network-layout.json";
    const auto destination = fs::temp_directory_path() / "network-layout.json";

    // Download the file.
    bool downloaded = downloadFile(fileUrl, destination, TEST_CACERT_PEM);
    BOOST_TEST(downloaded);
    BOOST_TEST(fs::exists(destination));

    // Check the content of the file.
    // We cannot check the whole file content as it changes over time,
    // but we can at least check some expected file properties.
    {
        const std::string expectedString = "\"stations\": [";
        std::ifstream file = destination;
        std::string line{};
        bool foundExpectedString = false;
        while (std::getline(file, line)) {
            if (line.find(expectedString) != std::string::npos) {
                foundExpectedString = true;
                break;
            }
        }
        BOOST_TEST(foundExpectedString);
    }

    // Clean up.
    fs::remove(destination);
}

BOOST_AUTO_TEST_CASE(test_parseJsonFile)
{
    const fs::path networkLayoutFile = TEST_NETWORK_LAYOUT;
    const auto networkLayout = parseJsonFile(networkLayoutFile);
    BOOST_TEST_REQUIRE(!networkLayout.is_discarded() );

    BOOST_TEST_REQUIRE(networkLayout.contains("lines") );
    BOOST_TEST_REQUIRE(networkLayout.at("lines").is_array() );
    BOOST_TEST_REQUIRE(!networkLayout.at("lines").empty() );

    BOOST_TEST_REQUIRE(networkLayout.contains("stations") );
    BOOST_TEST_REQUIRE(networkLayout.at("stations").is_array() );
    BOOST_TEST_REQUIRE(!networkLayout.at("stations").empty() );

    BOOST_TEST_REQUIRE(networkLayout.contains("travel_times") );
    BOOST_TEST_REQUIRE(networkLayout.at("travel_times").is_array() );
    BOOST_TEST_REQUIRE(!networkLayout.at("travel_times").empty() );
}

BOOST_AUTO_TEST_SUITE_END();
