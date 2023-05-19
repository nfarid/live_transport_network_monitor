
#include <network_monitor/stomp_frame.hpp>

#include <boost/test/unit_test.hpp>

#include <optional>
#include <string>

using NetworkMonitor::StompCommand;
using NetworkMonitor::StompError;
using NetworkMonitor::StompFrame;
using NetworkMonitor::StompHeader;

using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(network_monitor);


BOOST_AUTO_TEST_SUITE(stomp_frame);


BOOST_AUTO_TEST_CASE(parse_well_formed)
{
    std::string plain {
        "MESSAGE\n"
        "subscription:<subscription_id>\n"
        "message-id:<message_id>\n"
        "destination:/passengers\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Message);
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Subscription), "<subscription_id>");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::MessageId), "<message_id>");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Destination), "/passengers");
    BOOST_CHECK_EQUAL(frame.getBody(), "Frame body");
}

BOOST_AUTO_TEST_CASE(parse_well_formed_content_length)
{
    std::string plain {
        "MESSAGE\n"
        "subscription:<subscription_id>\n"
        "message-id:<message_id>\n"
        "destination:/passengers\n"
        "content-length:10\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Message);
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Subscription), "<subscription_id>");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::MessageId), "<message_id>");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Destination), "/passengers");
    BOOST_CHECK_EQUAL(frame.getBody(), "Frame body");
}

BOOST_AUTO_TEST_CASE(parse_empty_body)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Stomp);
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::AcceptVersion), "42");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Host), "host.com");
    BOOST_CHECK_EQUAL(frame.getBody().size(), 0);
}

BOOST_AUTO_TEST_CASE(parse_empty_body_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:0\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Stomp);
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::AcceptVersion), "42");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Host), "host.com");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::ContentLength), "0");
    BOOST_CHECK_EQUAL(frame.getBody().size(), 0);
}

BOOST_AUTO_TEST_CASE(parse_empty_headers)
{
    std::string plain {
        "ERROR\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Error);
    BOOST_CHECK_EQUAL(frame.getBody(), "Frame body");
}

BOOST_AUTO_TEST_CASE(parse_only_command)
{
    std::string plain {
        "DISCONNECT\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Disconnect);
    BOOST_CHECK_EQUAL(frame.getBody().size(), 0);
}

BOOST_AUTO_TEST_CASE(parse_bad_command)
{
    std::string plain {
        "CONNECTX\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_bad_header)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "login\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_missing_body_newline)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_missing_last_header_newline)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com"
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_unrecognized_header)
{
    std::string plain {
        "CONNECT\n"
        "bad_header:42\n"
        "host:host.com\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_empty_header_value)
{
    //STOMP 1.2 allows empty header values
    std::string plain {
        "CONNECT\n"
        "accept-version:\n"
        "host:host.com\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error == StompError::Ok);
}

BOOST_AUTO_TEST_CASE(parse_just_command)
{
    std::string plain {
        "CONNECT"
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_newline_after_command)
{
    std::string plain {
        "ERROR\n"
        "\n"
        "version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Error);

    // Everything becomes part of the body.
    BOOST_CHECK_EQUAL(frame.getBody().substr(0, 10), "version:42");
}

BOOST_AUTO_TEST_CASE(parse_double_colon_in_header_line)
{
    //STOMP 1.2 disallows colons in header values
    std::string plain {
        "CONNECT\n"
        "accept-version:42:43\n"
        "host:host.com\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_repeated_headers)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "accept-version:43\n"
        "host:host.com\n"
        "\n"
        "\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Stomp);
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::AcceptVersion), "42");
}

BOOST_AUTO_TEST_CASE(parse_missing_headers)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "accept-version:43\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error == StompError::Validation);
}

BOOST_AUTO_TEST_CASE(parse_unterminated_body)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_unterminated_body_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_junk_after_body)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "\n"
        "Frame body\0\n\njunk\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_junk_after_body_content_length)
{
    std::string plain {
        "CONNECT\n"
        "accept-version:42\n"
        "host:host.com\n"
        "content-length:10\n"
        "\n"
        "Frame body\0\n\njunk\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Parsing);
}

BOOST_AUTO_TEST_CASE(parse_newlines_after_body)
{
    std::string plain {
        "MESSAGE\n"
        "subscription:<subscription_id>\n"
        "message-id:<message_id>\n"
        "destination:/passengers\n"
        "\n"
        "Frame body\0\n\n\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Message);
    BOOST_CHECK_EQUAL(frame.getBody(), "Frame body");
}

BOOST_AUTO_TEST_CASE(parse_newlines_after_body_content_length)
{
    std::string plain {
        "MESSAGE\n"
        "subscription:<subscription_id>\n"
        "message-id:<message_id>\n"
        "destination:/passengers\n"
        "content-length:10\n"
        "\n"
        "Frame body\0\n\n\n"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(frame.getCommand(), StompCommand::Message);
    BOOST_CHECK_EQUAL(frame.getBody(), "Frame body");
}

BOOST_AUTO_TEST_CASE(parse_content_length_wrong_number)
{
    std::string plain {
        "MESSAGE\n"
        "subscription:<subscription_id>\n"
        "message-id:<message_id>\n"
        "destination:/passengers\n"
        "content-length:9\n" // This is one byte off
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Validation);
}

BOOST_AUTO_TEST_CASE(parse_content_length_exceeding)
{
    std::string plain {
        "MESSAGE\n"
        "subscription:<subscription_id>\n"
        "message-id:<message_id>\n"
        "destination:/passengers\n"
        "content-length:15\n" // Way above the actual body length
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, std::move(plain)};
    BOOST_TEST(error != StompError::Ok);
    BOOST_CHECK_EQUAL(error, StompError::Validation);
}

BOOST_AUTO_TEST_CASE(parse_required_headers)
{
    StompError error;
    {
        std::string plain {
            "CONNECT\n"
            "\n"
            "\0"s
        };
        StompFrame frame {error, std::move(plain)};
        BOOST_TEST(error != StompError::Ok);
        BOOST_CHECK_EQUAL(error, StompError::Validation);
    }
    {
        std::string plain {
            "CONNECT\n"
            "accept-version:42\n"
            "\n"
            "\0"s
        };
        StompFrame frame {error, std::move(plain)};
        BOOST_TEST(error != StompError::Ok);
        BOOST_CHECK_EQUAL(error, StompError::Validation);
    }
    {
        std::string plain {
            "CONNECT\n"
            "accept-version:42\n"
            "host:host.com\n"
            "\n"
            "\0"s
        };
        StompFrame frame {error, std::move(plain)};
        BOOST_TEST_REQUIRE(error == StompError::Ok);
    }
}

BOOST_AUTO_TEST_CASE(copy_constructors)
{
    // Check that the std::string_view's stay valid when copied object goes out of scope.
    std::optional<StompFrame> assigned;
    {
        std::string plain {
            "MESSAGE\n"
            "subscription:<subscription_id>\n"
            "message-id:<message_id>\n"
            "destination:/passengers\n"
            "\n"
            "Frame body\0"s
        };
        StompError error;
        StompFrame newFrame(error, std::move(plain));
        BOOST_REQUIRE(error == StompError::Ok);
        assigned = newFrame;
    }
    BOOST_TEST(assigned->getCommand() == StompCommand::Message);
    BOOST_CHECK_EQUAL(assigned->getHeader(StompHeader::Subscription), "<subscription_id>");
    BOOST_CHECK_EQUAL(assigned->getHeader(StompHeader::MessageId), "<message_id>");
    BOOST_CHECK_EQUAL(assigned->getHeader(StompHeader::Destination), "/passengers");
    BOOST_CHECK_EQUAL(assigned->getBody(), "Frame body");
}

BOOST_AUTO_TEST_CASE(constructor_from_components_full)
{
    StompError error;
    StompFrame frame {
        error,
        StompCommand::Message,
        {
            {StompHeader::Subscription, "<subscription_id>"},
            {StompHeader::MessageId, "<message_id>"},
            {StompHeader::Destination, "/passengers"},
        },
        "Frame body"
    };
    BOOST_TEST(error == StompError::Ok);
    BOOST_TEST(frame.getCommand() == StompCommand::Message);
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Subscription), "<subscription_id>");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::MessageId), "<message_id>");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Destination), "/passengers");
    BOOST_CHECK_EQUAL(frame.getBody(), "Frame body");
}

BOOST_AUTO_TEST_CASE(constructor_from_components_only_command)
{
    StompError error;
    StompFrame frame {
        error,
        StompCommand::Disconnect,
    };
    BOOST_TEST(error == StompError::Ok);
    BOOST_TEST(frame.getCommand() == StompCommand::Disconnect);
}

BOOST_AUTO_TEST_CASE(constructor_from_components_empty_body)
{
    StompError error;
    StompFrame frame {
        error,
        StompCommand::Stomp,
        {
            {StompHeader::AcceptVersion, "42"},
            {StompHeader::Host, "host.com"},
        }
    };
    BOOST_TEST(error == StompError::Ok);
    BOOST_TEST(frame.getCommand() == StompCommand::Stomp);
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::AcceptVersion), "42");
    BOOST_CHECK_EQUAL(frame.getHeader(StompHeader::Host), "host.com");
}

BOOST_AUTO_TEST_CASE(to_string)
{
    const std::string plain {
        "MESSAGE\n"
        "subscription:<subscription_id>\n"
        "message-id:<message_id>\n"
        "destination:/passengers\n"
        "\n"
        "Frame body\0"s
    };
    StompError error;
    StompFrame frame {error, plain};
    BOOST_TEST_REQUIRE(error == StompError::Ok);
    BOOST_CHECK_EQUAL(plain, frame.toString());
}


BOOST_AUTO_TEST_SUITE_END(); // stomp_frame


BOOST_AUTO_TEST_SUITE_END(); // network_monitor
