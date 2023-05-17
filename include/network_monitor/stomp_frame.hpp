
#ifndef HPP_NETWORKMONITOR_STOMPFRAME_
#define HPP_NETWORKMONITOR_STOMPFRAME_

#include <iosfwd>
#include <string>

namespace NetworkMonitor  {

/*! \brief Available STOMP commands, from the STOMP protocol v1.2.
 */
enum class StompCommand : unsigned {
    // TODO: Your enum values go here
    // ...
};

std::ostream& operator<<(std::ostream& os, StompCommand sc);

// ...

/*! \brief Available STOMP headers, from the STOMP protocol v1.2.
 */
enum class StompHeader : unsigned {
    // TODO: Your enum values go here
    // ...
};

std::ostream& operator<<(std::ostream& os, StompHeader sh);

// ...

/*! \brief Error codes for the STOMP protocol
 */
enum class StompError : unsigned {
    // TODO: Your enum values go here
    // ...
    Ok = 0,
};

std::ostream& operator<<(std::ostream& os, StompError se);

// ...

/* \brief STOMP frame representation, supporting STOMP v1.2.
 */
class StompFrame {
public:
    /*! \brief Default constructor. Corresponds to an empty, invalid STOMP frame.
     */
    StompFrame();

    /*! \brief Construct the STOMP frame from a string. The string is copied.
     *
     *  The result of the operation is stored in the error code.
     */
    explicit StompFrame(StompError& ec, const std::string& frame);

    /*! \brief Construct the STOMP frame from a string. The string is moved into the object.
     *
     *  The result of the operation is stored in the error code.
     */
    explicit StompFrame(StompError& ec, std::string&& frame);

    // TODO: Other APIs go here
    // ...
};

} // namespace NetworkMonitor

#endif // HPP_NETWORKMONITOR_STOMPFRAME_
