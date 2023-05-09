#ifndef HPP_NETWORKMONITOR_DOWNLOADFILE_
#define HPP_NETWORKMONITOR_DOWNLOADFILE_

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace NetworkMonitor {

/*! \brief Download a file from a remote HTTPS URL.
 *
 *  \param fileURl      The url to download the file form.
 *  \param destination  The full path of the output file, its directory must exist.
 *  \param caCertFile   The path to a cacert.pem file to perform certificate verification.
 *
 *  \return true if successful, false otherwise
 */
bool downloadFile(
    const std::string& fileURL,
    const std::filesystem::path& destination,
    const std::filesystem::path& cacertFile = ""
);


/*! \brief Parse a local file into a JSON object.
 *
 *  \param source   The path to the JSON file to load and parse, must exist.
 */
nlohmann::json parseJsonFile(const std::filesystem::path& source);

} //NetworkMonitor

#endif // HPP_NETWORKMONITOR_DOWNLOADFILE_
