
#include <network_monitor/stomp_frame.hpp>

#include <stdexcept>
#include <string>

using std::string_literals::operator""s;

namespace NetworkMonitor {

std::ostream& operator<<(std::ostream& os, StompCommand sc) {
    throw std::logic_error("Not implemented: "s + __func__);
}

std::ostream& operator<<(std::ostream& os, StompHeader sh) {
    throw std::logic_error("Not implemented: "s + __func__);
}

std::ostream& operator<<(std::ostream& os, StompError se) {
    throw std::logic_error("Not implemented: "s + __func__);
}

StompFrame::StompFrame() {
    throw std::logic_error("Not implemented: "s + __func__);
}

StompFrame::StompFrame(StompError& ec, const std::string& frame) {
    throw std::logic_error("Not implemented: "s + __func__);
}

StompFrame::StompFrame(StompError& ec, std::string&& frame) {
    throw std::logic_error("Not implemented: "s + __func__);
}

} //namespace NetworkMonitor
