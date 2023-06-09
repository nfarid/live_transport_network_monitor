
#include <network_monitor/stomp_frame.hpp>

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/spirit/home/x3.hpp>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <string_view>
#include <tuple>
#include <utility>

#include <type_traits>


using std::string_literals::operator""s, std::string_view_literals::operator""sv;

namespace NetworkMonitor {

namespace  {

const std::array<std::string_view, StompCommand_count> stompCommandLookup = {
    "CONNECTED",
    "DISCONNECT",
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


std::string unEscapeString(std::string&& str) {
    bool isEscape = false;
    size_t j = 0;
    for(size_t i=0; i<str.size(); ++i) {
        if(str[i] == '\\') {
            isEscape = true;
            continue;
        }
        if(isEscape) {
            isEscape = false;
            if(str[i] == 'n')
                str[j] = '\n';
            else if(str[i] == 'r')
                str[j] = '\r';
            else if(str[i] == ':')
                str[j] = ':';
            else
                throw std::runtime_error("Invalid escape character: \\"s + str[i]);
        } else {
            str[j] = str[i];
        }
        ++j;
    }
    str.resize(j);
    return str;
}

std::string escapeString(std::string_view str) {
    std::string ret;
    ret.reserve(str.size() );
    for(const char c : str) {
        if(c == '\n') {
            ret.push_back('\\');
            ret.push_back('n');
        } else if(c == '\r') {
            ret.push_back('\\');
            ret.push_back('r');
        } else if(c == ':') {
            ret.push_back('\\');
            ret.push_back(':');
        } else if(c == '\\') {
            ret.push_back('\\');
            ret.push_back('\\');
        } else {
            ret.push_back(c);
        }
    }
    return ret;
}


bool contains(std::initializer_list<StompCommand> lst, StompCommand sc) {
    for(const auto& elem : lst) {
        if(elem == sc)
            return true;
    }
    return false;
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

StompFrame::StompFrame(StompError& ec,
                       StompCommand sc,
                       std::unordered_map<StompHeader, std::string>&& headerMp,
                       std::string body) :
    m_command{sc},
    m_headerMp{std::move(headerMp)},
    m_body{std::move(body)}
{
    ec = validation();
    if(ec != StompError::Ok)
        return;
    std::stringstream ss;
    ss << m_command << '\n';
    for(const auto& [k,v] : m_headerMp) {
        if(m_command == StompCommand::Stomp || m_command == StompCommand::Connected)
            ss << k << ':' << v << '\n';
        else
            ss << k << ':' << escapeString(v) << '\n';
    }
    ss << '\n';
    ss << m_body;
    ss << '\0';
    m_frame = ss.str();
}

StompCommand StompFrame::getCommand() const {
    return m_command;
}

bool StompFrame::hasHeader(StompHeader sh) const {
    return m_headerMp.contains(sh);
}

std::string_view StompFrame::getHeader(StompHeader sh) const {
    if(!m_headerMp.contains(sh) )
        throw std::invalid_argument("Unable to find header: " + std::string(toStringView(sh) ) );
    return m_headerMp.at(sh);
}

std::string_view StompFrame::getBody() const {
    return m_body;
}

std::string StompFrame::toString() const {
    return m_frame;
}

StompError StompFrame::parseFrame() {
    namespace Parser = boost::spirit::x3;
    const auto eol = -Parser::lit('\r') >> '\n';

    constexpr auto clientCommand = Parser::string("SEND\n")
                                 | Parser::string("SUBSCRIBE\n")
                                 | Parser::string("UNSUBSCRIBE\n")
                                 | Parser::string("BEGIN\n")
                                 | Parser::string("COMMIT\n")
                                 | Parser::string("ABORT\n")
                                 | Parser::string("ACK\n")
                                 | Parser::string("NACK\n")
                                 | Parser::string("DISCONNECT\n")
                                 | Parser::string("CONNECT\n")
                                 | Parser::string("STOMP\n");
    constexpr auto serverCommand = Parser::string("CONNECTED\n")
                                 | Parser::string("MESSAGE\n")
                                 | Parser::string("RECEIPT\n")
                                 | Parser::string("ERROR\n");
    const auto command = (clientCommand | serverCommand);

    const auto invalidHeaderChar = Parser::lit(':') | '\r' | '\n';
    const auto headerChar = Parser::char_ - invalidHeaderChar;
    const auto headerName = +headerChar;
    const auto headerValue = *headerChar;
    const auto header = (headerName >> ':' >> headerValue);

    const auto frame = command
                        >> *(header >> eol)
                        >> eol
                        >> *Parser::char_;


    std::tuple<std::string, std::vector<std::pair<std::string, std::string> >, std::string> parsed;
    if(!Parser::parse(m_frame.cbegin(), m_frame.cend(), frame, parsed) )
       return StompError::Parsing;

    try {
        std::get<0>(parsed).pop_back();
        m_command = toStompCommand(std::get<0>(parsed) );

        auto& headerLst = std::get<1>(parsed);
        for(auto iter = headerLst.rbegin(); iter != headerLst.rend(); ++iter) {
            auto headerName = toStompHeader(iter->first);
            if(m_command == StompCommand::Stomp || m_command == StompCommand::Connected)
                m_headerMp[headerName] = std::move(iter->second);
            else
                m_headerMp[headerName] = unEscapeString(std::move(iter->second) );
        }
    } catch (std::runtime_error& ex) {
        std::cerr<<__func__<<":"<<__LINE__<<" : "<<ex.what()<<std::endl;
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

    return validation();
}

StompError StompFrame::validation() {
    //Check body
    if(!contains({StompCommand::Send, StompCommand::Message, StompCommand::Error}, m_command) && !m_body.empty() )
        return StompError::Validation;
    if(m_headerMp.contains(StompHeader::ContentLength) ) {
        const auto lenStr = m_headerMp[StompHeader::ContentLength];
        try {
            const int len = std::stoi(lenStr);
            if(m_body.length() != len)
                return StompError::Validation;
        }  catch (const std::exception& ex) {
            std::cerr<<__func__<<":"<<__LINE__<<" : "<<ex.what()<<std::endl;
            return StompError::Parsing;
        }
    }

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
    case StompCommand::Disconnect:
    case StompCommand::Error:
    break;
    }

    return StompError::Ok;
}

} //namespace NetworkMonitor
