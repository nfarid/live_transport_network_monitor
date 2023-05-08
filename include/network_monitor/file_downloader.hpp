#ifndef HPP_NETWORKMONITOR_DOWNLOADFILE_
#define HPP_NETWORKMONITOR_DOWNLOADFILE_

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

} //NetworkMonitor

#endif // HPP_NETWORKMONITOR_DOWNLOADFILE_
