
#include <network_monitor/stomp_frame.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>


using std::string_literals::operator""s, std::string_view_literals::operator""sv;

namespace NetworkMonitor {

namespace  {

std::string to_string(StompCommand sc) {
    switch(sc) {
    case StompCommand::Stomp:
        return "STOMP";
    case StompCommand::Connected:
        return "CONNECTED";
    case StompCommand::Error:
        return "ERROR";
    case StompCommand::Subscribe:
        return "SUBSCRIBE";
    case StompCommand::Receipt:
        return "RECEIPT";
    case StompCommand::Message:
        return "MESSAGE";
    case StompCommand::Send:
        return "SEND";
    }
    throw std::logic_error("Unreachable: "s + __func__);
}

StompCommand toStompCommand(std::string_view str) {
    if(str == "STOMP" || str == "CONNECT")
        return StompCommand::Stomp;
    else if(str == "CONNECTED")
        return StompCommand::Connected;
    else if(str == "ERROR")
        return StompCommand::Error;
    else if(str == "SUBSCRIBE")
        return StompCommand::Subscribe;
    else if(str == "RECEIPT")
        return StompCommand::Receipt;
    else if(str == "MESSAGE")
        return StompCommand::Message;
    else if(str == "SEND")
        return StompCommand::Message;
    else
        throw std::runtime_error("Invalid stomp command: "s + std::string(str) );
}

std::string to_string(StompHeader sh) {
    switch(sh) {
    case StompHeader::AcceptVersion:
        return "accept-version";
    case StompHeader::Host:
        return "host";
    case StompHeader::Login:
        return "login";
    case StompHeader::Passcode:
        return "passcode";
    case StompHeader::Version:
        return "version";
    case StompHeader::Session:
        return "session";
    case StompHeader::ContentLength:
        return "content-length";
    case StompHeader::ContentType:
        return "content-type";
    case StompHeader::Receipt:
        return "receipt";
    case StompHeader::Destination:
        return "destination";
    case StompHeader::Ack:
        return "ack";
    case StompHeader::ReceiptId:
        return "receipt-id";
    case StompHeader::MessageId:
        return "message-id";
    case StompHeader::Id:
        return "id";
    }
    throw std::logic_error("Unreachable: "s + __func__);
}

StompHeader toStompHeader(std::string_view str) {
    if(str == "accept-version")
        return StompHeader::AcceptVersion;
    else if(str =="host")
        return StompHeader::Host;
    else if(str =="login")
        return StompHeader::Login;
    else if(str =="passcode")
        return StompHeader::Passcode;
    else if(str =="version")
        return StompHeader::Version;
    else if(str =="session")
        return StompHeader::Session;
    else if(str =="content-length")
        return StompHeader::ContentLength;
    else if(str =="content-type")
        return StompHeader::ContentType;
    else if(str =="receipt")
        return StompHeader::Receipt;
    else if(str =="destination")
        return StompHeader::Destination;
    else if(str =="ack")
        return StompHeader::Ack;
    else if(str =="receipt-id")
        return StompHeader::ReceiptId;
    else if(str =="message-id")
        return StompHeader::MessageId;
    else if(str =="id")
        return StompHeader::Id;
    else
        throw std::runtime_error("Invalid stomp header: "s + std::string(str) );
}

} //namespace

std::ostream& operator<<(std::ostream& os, StompCommand sc) {
    return os<<to_string(sc);
}

std::ostream& operator<<(std::ostream& os, StompHeader sh) {
    return os<<to_string(sh);
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
