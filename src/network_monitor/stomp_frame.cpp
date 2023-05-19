
#include <network_monitor/stomp_frame.hpp>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>


using std::string_literals::operator""s, std::string_view_literals::operator""sv;

namespace NetworkMonitor {

namespace  {

const std::array<std::string_view, StompCommand_count> stompCommandLookup = {
    "CONNECTED",
    "ERROR",
    "MESSAGE",
    "RECEIPT",
    "SEND",
    "STOMP",
    "SUBSCRIBE",
};

std::string_view toStringView(StompCommand sc) {
    return stompCommandLookup[static_cast<size_t>(sc)];
}

StompCommand toStompCommand(std::string_view sc) {
    for(size_t i=0; i<size(stompCommandLookup); ++i) {
        if(stompCommandLookup[i] == sc)
            return static_cast<StompCommand>(i);
    }
    if(sc == "CONNECT")
        return StompCommand::Stomp;
    throw std::runtime_error("Invalid stomp command: "s + std::string(sc) );
}

const std::array<std::string_view, StompHeader_count> stompHeaderLookup = {
    "accept-version",
    "ack",
    "content-length",
    "content-type",
    "destination",
    "host",
    "id",
    "login",
    "message-id",
    "passcode",
    "receipt",
    "receipt-id",
    "session",
    "subscription",
    "version",
};

std::string_view toStringView(StompHeader sh) {
    return stompHeaderLookup[static_cast<size_t>(sh)];
}

StompHeader toStompHeader(std::string_view sh) {
    for(size_t i=0; i<size(stompHeaderLookup); ++i) {
        if(stompHeaderLookup[i] == sh)
            return static_cast<StompHeader>(i);
    }
    throw std::runtime_error("Invalid stomp header: "s + std::string(sh) );
}

} //namespace

std::ostream& operator<<(std::ostream& os, StompCommand sc) {
    return os<<toStringView(sc);
}

std::ostream& operator<<(std::ostream& os, StompHeader sh) {
    return os<<toStringView(sh);
}

std::ostream& operator<<(std::ostream& os, StompError se) {
    switch(se) {
    case StompError::Ok:
        return os<<"OK";
    case StompError::Parsing:
        return os<<"Parsing Error";
    case StompError::Validation:
        return os<<"Validation Error";
    }
    throw std::logic_error("Unreachable: "s + __func__);

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
