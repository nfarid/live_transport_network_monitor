
#ifndef HPP_NETWORKMONITOR_STOMPFRAME_
#define HPP_NETWORKMONITOR_STOMPFRAME_

#include <cstddef>
#include <initializer_list>
#include <iosfwd>
#include <string>
#include <string_view>
#include <unordered_map>

using std::size_t;

namespace NetworkMonitor  {

/*! \brief Available STOMP commands, from the STOMP protocol v1.2.
 */
enum class StompCommand : unsigned {
    Connected,
    Disconnect,
    Error,
    Message,
    Receipt,
    Send,
    Stomp,
    Subscribe,
};
constexpr size_t StompCommand_count = 8;

std::ostream& operator<<(std::ostream& os, StompCommand sc);

// ...

/*! \brief Available STOMP headers, from the STOMP protocol v1.2.
 */
enum class StompHeader : unsigned {
    AcceptVersion,
    Ack,
    ContentLength,
    ContentType,
    Destination,
    Host,
    Id,
    Login,
    MessageId,
    Passcode,
    Receipt,
    ReceiptId,
    Session,
    Subscription,
    Version,
};
constexpr size_t StompHeader_count = 15;

std::ostream& operator<<(std::ostream& os, StompHeader sh);

// ...

/*! \brief Error codes for the STOMP protocol
 */
enum class StompError : unsigned {
    Ok,
    Parsing,
    Validation,
};

std::ostream& operator<<(std::ostream& os, StompError se);

// ...

/* \brief STOMP frame representation, supporting STOMP v1.2.
 */
class StompFrame {
public:
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

    /*! \brief Construct the STOMP frame from command, headers and body
     *
     *  The result of the operation is stored in the error code.
     */
    /*implicit*/ StompFrame(StompError& ec,
                            StompCommand sc,
                            std::unordered_map<StompHeader, std::string>&& headerMp = {},
                            std::string body = "");

    StompCommand getCommand() const;
    bool hasHeader(StompHeader sh) const;
    std::string_view getHeader(StompHeader sh) const;
    std::string_view getBody() const;
    std::string toString() const;

private:
    std::string m_frame{};
    StompCommand m_command{};
    std::unordered_map<StompHeader, std::string> m_headerMp{};
    std::string m_body{};

    StompError parseFrame();
    StompError validation();
};

} // namespace NetworkMonitor

#endif // HPP_NETWORKMONITOR_STOMPFRAME_
