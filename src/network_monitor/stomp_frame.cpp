
#include <network_monitor/stomp_frame.hpp>

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/spirit/home/x3.hpp>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include <type_traits>


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

StompFrame::StompFrame(StompError& ec, const std::string& frame) :
    m_frame(frame)
{
    ec = parseFrame();
}

StompFrame::StompFrame(StompError& ec, std::string&& frame) :
    m_frame(std::move(frame) )
{
    ec = parseFrame();
}

StompError StompFrame::parseFrame() {
    namespace Parser = boost::spirit::x3;
    const auto eol = -Parser::lit('\r') >> '\n';

    constexpr auto clientCommand = Parser::string("SEND")
                                 | Parser::string("SUBSCRIBE")
                                 | Parser::string("UNSUBSCRIBE")
                                 | Parser::string("BEGIN")
                                 | Parser::string("COMMIT")
                                 | Parser::string("ABORT")
                                 | Parser::string("ACK")
                                 | Parser::string("NACK")
                                 | Parser::string("DISCONNECT")
                                 | Parser::string("CONNECT")
                                 | Parser::string("STOMP");
    constexpr auto serverCommand = Parser::string("CONNECTED")
                                 | Parser::string("MESSAGE")
                                 | Parser::string("RECEIPT")
                                 | Parser::string("ERROR");
    const auto command = (clientCommand | serverCommand);

    const auto invalidHeaderChar = Parser::lit(':') | '\r' | '\n';
    const auto headerChar = Parser::char_ - invalidHeaderChar;
    const auto headerName = +headerChar;
    const auto headerValue = *headerChar;
    const auto header = (headerName >> ':' >> headerValue);

    const auto frame = command >> eol
                        >> *(header >> eol)
                        >> eol
                        >> *Parser::char_;


   std::tuple<std::string, std::vector<std::pair<std::string, std::string> >, std::string> parsed;
   if(!Parser::parse(m_frame.cbegin(), m_frame.cend(), frame, parsed) )
       return StompError::Parsing;

    try {
        m_command = toStompCommand(std::get<0>(parsed) );

        auto& headerLst = std::get<1>(parsed);
        for(auto iter = headerLst.rbegin(); iter != headerLst.rend(); ++iter) {
            auto headerName = toStompHeader(iter->first);
            //TODO: escape strings
            m_headerMp[headerName] = std::move(iter->second);
        }
    } catch (std::runtime_error& ex) {
        std::cerr<<ex.what()<<std::endl;
        return StompError::Parsing;
    }

    m_body = std::get<2>(parsed);
    while(!m_body.empty() ) {
        if(m_body.back() == '\n') {
            m_body.pop_back();
            if(!m_body.empty() && m_body.back() == '\r')
                m_body.pop_back();
        } else if(m_body.back() == '\0') {
            break;
        } else {
            return StompError::Parsing;
        }
    }
    if(m_body.empty() )
        return StompError::Parsing;
    m_body.pop_back();

    if(m_headerMp.contains(StompHeader::ContentLength) ) {
        const auto lenStr = m_headerMp[StompHeader::ContentLength];
        try {
            const int len = std::stoi(lenStr);
            if(m_body.length() < len)
                return StompError::Validation;
            m_body.resize(len);
        }  catch (const std::exception& ex) {
            std::cerr<<ex.what()<<std::endl;
            return StompError::Parsing;
        }
    }
    //StompHeader::

    //Check required headers
    switch(m_command) {
    case StompCommand::Stomp:
        if(!m_headerMp.contains(StompHeader::AcceptVersion) ||
           !m_headerMp.contains(StompHeader::Host) )
        {
            return StompError::Validation;
        }
    break;
    case StompCommand::Connected:
        if(!m_headerMp.contains(StompHeader::Version) )
            return StompError::Validation;
    break;
    case StompCommand::Send:
        if(!m_headerMp.contains(StompHeader::Destination) )
            return StompError::Validation;
    break;
    case StompCommand::Subscribe:
        if(!m_headerMp.contains(StompHeader::Destination) ||
           !m_headerMp.contains(StompHeader::Id) )
        {
            return StompError::Validation;
        }
    break;
    case StompCommand::Receipt:
        if(!m_headerMp.contains(StompHeader::ReceiptId) )
            return StompError::Validation;
    break;
    case StompCommand::Message:
        if(!m_headerMp.contains(StompHeader::Destination) ||
           !m_headerMp.contains(StompHeader::MessageId)   ||
           !m_headerMp.contains(StompHeader::Subscription) )
        {
            return StompError::Validation;
        }
    break;
    case StompCommand::Error:
    break;
    }


    //TOOD: validation
    return StompError::Ok;
}

} //namespace NetworkMonitor
