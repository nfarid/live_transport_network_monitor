
#ifndef HPP_NETWORKMONITOR_STOMPFRAME_
#define HPP_NETWORKMONITOR_STOMPFRAME_

#include <iosfwd>
#include <string>

namespace NetworkMonitor  {

/*! \brief Available STOMP commands, from the STOMP protocol v1.2.
 */
enum class StompCommand : unsigned {
    Stomp,
    Connected,
    Error,
    Subscribe,
    Receipt,
    Message,
    Send,
};

std::ostream& operator<<(std::ostream& os, StompCommand sc);

// ...

/*! \brief Available STOMP headers, from the STOMP protocol v1.2.
 */
enum class StompHeader : unsigned {
    AcceptVersion,
    Host,
    Login,
    Passcode,
    Version,
    Session,
    ContentLength,
    ContentType,
    Receipt,
    Destination,
    Ack,
    ReceiptId,
    MessageId,
    Id,
};

std::ostream& operator<<(std::ostream& os, StompHeader sh);

// ...

/*! \brief Error codes for the STOMP protocol
 */
enum class StompError : unsigned {
    Ok = 0,
    Parsing,
    Validation,
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
