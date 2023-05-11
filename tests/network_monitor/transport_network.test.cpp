#include <network_monitor/transport_network.hpp>

#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

using NetworkMonitor::Id;
using NetworkMonitor::Line;
using NetworkMonitor::PassengerEvent;
using NetworkMonitor::Route;
using NetworkMonitor::Station;
using NetworkMonitor::TransportNetwork;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(class_TransportNetwork);

BOOST_AUTO_TEST_SUITE(AddStation);

BOOST_AUTO_TEST_CASE(basic)
{
    TransportNetwork nw {};
    bool ok {false};

    // Add a station.
    Station station {
        "station_000",
        "Station Name",
    };
    ok = nw.addStation(station);
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(duplicate_id)
{
    TransportNetwork nw {};
    bool ok {false};

    // Can't add the same station twice.
    Station station {
        "station_000",
        "Station Name",
    };
    ok = nw.addStation(station);
    BOOST_REQUIRE(ok);
    ok = nw.addStation(station);
    BOOST_CHECK(!ok);
}

BOOST_AUTO_TEST_CASE(duplicate_name)
{
    TransportNetwork nw {};
    bool ok {false};

    // It's ok to add a station with the same name, but different ID.
    Station station0 {
        "station_000",
        "Same Name",
    };
    ok = nw.addStation(station0);
    BOOST_REQUIRE(ok);
    Station station1 {
        "station_001",
        "Same Name",
    };
    ok = nw.addStation(station1);
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_SUITE_END(); // AddStation

BOOST_AUTO_TEST_SUITE(AddLine);

BOOST_AUTO_TEST_CASE(basic)
{
    TransportNetwork nw {};
    bool ok {false};

    // Add a line with 1 route.
    // route0: 0 ---> 1

    // First, add the stations.
    Station station0 {
        "station_000",
        "Station Name 0",
    };
    Station station1 {
        "station_001",
        "Station Name 1",
    };
    ok = true;
    ok &= nw.addStation(station0);
    ok &= nw.addStation(station1);
    BOOST_REQUIRE(ok);

    // Then, add the line, with the two routes.
    Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_001",
        {"station_000", "station_001"},
    };
    Line line {
        "line_000",
        "Line Name",
        {route0},
    };
    ok = nw.addLine(line);
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(shared_stations)
{
    TransportNetwork nw {};
    bool ok {false};

    // Define a line with 2 routes going through some shared stations.
    // route0: 0 ---> 1 ---> 2
    // route1: 3 ---> 1 ---> 2
    Station station0 {
        "station_000",
        "Station Name 0",
    };
    Station station1 {
        "station_001",
        "Station Name 1",
    };
    Station station2 {
        "station_002",
        "Station Name 2",
    };
    Station station3 {
        "station_003",
        "Station Name 3",
    };
    Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_002",
        {"station_000", "station_001", "station_002"},
    };
    Route route1 {
        "route_001",
        "inbound",
        "line_000",
        "station_003",
        "station_002",
        {"station_003", "station_001", "station_002"},
    };
    Line line {
        "line_000",
        "Line Name",
        {route0, route1},
    };
    ok = true;
    ok &= nw.addStation(station0);
    ok &= nw.addStation(station1);
    ok &= nw.addStation(station2);
    ok &= nw.addStation(station3);
    BOOST_REQUIRE(ok);
    ok = nw.addLine(line);
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(duplicate)
{
    TransportNetwork nw {};
    bool ok {false};

    // Can't add the same line twice.
    Station station0 {
        "station_000",
        "Station Name 0",
    };
    Station station1 {
        "station_001",
        "Station Name 1",
    };
    Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_001",
        {"station_000", "station_001"},
    };
    Line line {
        "line_000",
        "Line Name",
        {route0},
    };
    ok = true;
    ok &= nw.addStation(station0);
    ok &= nw.addStation(station1);
    BOOST_REQUIRE(ok);
    ok = nw.addLine(line);
    BOOST_REQUIRE(ok);
    ok = nw.addLine(line);
    BOOST_CHECK(!ok);
}

BOOST_AUTO_TEST_CASE(missing_stations)
{
    TransportNetwork nw{};

    // Define a line with 1 route.
    // route0: 0 ---> 1 ---> 2
    const Station station0 {
        "station_000",
        "Station Name 0",
    };
    const Station station1 {
        "station_001",
        "Station Name 1",
    };
    const Station station2 {
        "station_002",
        "Station Name 2",
    };
    const Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_002",
        {"station_000", "station_001", "station_002"},
    };
    const Line line {
        "line_000",
        "Line Name",
        {route0},
    };

    // Expected fail: No stations in the network.
    BOOST_TEST(!nw.addLine(line) );

    // Expected fail: We add all stations except one.
    BOOST_TEST_REQUIRE(nw.addStation(station0) );
    BOOST_TEST_REQUIRE(nw.addStation(station1) );
    BOOST_TEST(!nw.addLine(line) );

    // Expected success: We add the final station and try again.
    BOOST_TEST_REQUIRE(nw.addStation(station2) );
    BOOST_TEST(nw.addLine(line) );
}

BOOST_AUTO_TEST_SUITE_END(); // AddLine

BOOST_AUTO_TEST_SUITE(PassengerEvents);

BOOST_AUTO_TEST_CASE(basic)
{
    TransportNetwork nw {};
    bool ok {false};

    // Add a line with 1 route.
    // route0: 0 ---> 1 ---> 2
    Station station0 {
        "station_000",
        "Station Name 0",
    };
    Station station1 {
        "station_001",
        "Station Name 1",
    };
    Station station2 {
        "station_002",
        "Station Name 2",
    };
    Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_002",
        {"station_000", "station_001", "station_002"},
    };
    Line line {
        "line_000",
        "Line Name",
        {route0},
    };
    ok = true;
    ok &= nw.addStation(station0);
    ok &= nw.addStation(station1);
    ok &= nw.addStation(station2);
    BOOST_REQUIRE(ok);
    ok = nw.addLine(line);
    BOOST_REQUIRE(ok);

    // Check that the network starts empty.
    BOOST_REQUIRE_EQUAL(nw.getPassengerCount(station0.id), 0);
    BOOST_REQUIRE_EQUAL(nw.getPassengerCount(station1.id), 0);
    BOOST_REQUIRE_EQUAL(nw.getPassengerCount(station2.id), 0);
    try {
        auto count {nw.getPassengerCount("station_42")}; // Not in the network
        BOOST_REQUIRE(false);
    } catch (const std::runtime_error& e) {
        BOOST_REQUIRE(true);
    }

    // Record events and check the count.
    using EventType = PassengerEvent::Type;
    ok = nw.recordPassengerEvent({station0.id, EventType::In});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station0.id), 1);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station1.id), 0);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station2.id), 0);
    ok = nw.recordPassengerEvent({station0.id, EventType::In});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station0.id), 2);
    ok = nw.recordPassengerEvent({station1.id, EventType::In});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station0.id), 2);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station1.id), 1);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station2.id), 0);
    ok = nw.recordPassengerEvent({station0.id, EventType::Out});
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station0.id), 1);
    ok = nw.recordPassengerEvent({station2.id, EventType::Out}); // Negative
    BOOST_REQUIRE(ok);
    BOOST_CHECK_EQUAL(nw.getPassengerCount(station2.id), -1);
}

BOOST_AUTO_TEST_SUITE_END(); // PassengerEvents

BOOST_AUTO_TEST_SUITE(GetRoutesServingStation);

BOOST_AUTO_TEST_CASE(basic)
{
    TransportNetwork nw {};
    bool ok {false};

    // Add a line with 1 route.
    // route0: 0 ---> 1 ---> 2
    // Plus a station served by no routes: 3.
    Station station0 {
        "station_000",
        "Station Name 0",
    };
    Station station1 {
        "station_001",
        "Station Name 1",
    };
    Station station2 {
        "station_002",
        "Station Name 2",
    };
    Station station3 {
        "station_003",
        "Station Name 3",
    };
    Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_002",
        {"station_000", "station_001", "station_002"},
    };
    Line line {
        "line_000",
        "Line Name",
        {route0},
    };
    ok = true;
    ok &= nw.addStation(station0);
    ok &= nw.addStation(station1);
    ok &= nw.addStation(station2);
    ok &= nw.addStation(station3);
    BOOST_REQUIRE(ok);
    ok = nw.addLine(line);
    BOOST_REQUIRE(ok);

    // Check the routes served.
    std::vector<Id> routes {};
    routes = nw.getRoutesServingStation(station0.id);
    BOOST_REQUIRE_EQUAL(routes.size(), 1);
    BOOST_CHECK(routes[0] == route0.id);
    routes = nw.getRoutesServingStation(station1.id);
    BOOST_REQUIRE_EQUAL(routes.size(), 1);
    BOOST_CHECK(routes[0] == route0.id);
    routes = nw.getRoutesServingStation(station2.id);
    BOOST_REQUIRE_EQUAL(routes.size(), 1);
    BOOST_CHECK(routes[0] == route0.id);
    routes = nw.getRoutesServingStation(station3.id);
    BOOST_CHECK_EQUAL(routes.size(), 0);
}

BOOST_AUTO_TEST_CASE(lone_station)
{
    TransportNetwork nw{};

    // Add a single station.
    const Station station0 {
        "station_000",
        "Station Name 0",
    };
    BOOST_TEST_REQUIRE(nw.addStation(station0) );

    // Check the routes served.
    const auto routes = nw.getRoutesServingStation(station0.id);
    BOOST_TEST(routes.empty() );
}


BOOST_AUTO_TEST_SUITE_END(); // GetRoutesServingStation

BOOST_AUTO_TEST_SUITE(TravelTime);

BOOST_AUTO_TEST_CASE(basic)
{
    TransportNetwork nw {};
    bool ok {false};

    // Add a line with 1 route.
    // route0: 0 ---> 1 ---> 2
    Station station0 {
        "station_000",
        "Station Name 0",
    };
    Station station1 {
        "station_001",
        "Station Name 1",
    };
    Station station2 {
        "station_002",
        "Station Name 2",
    };
    Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_002",
        {"station_000", "station_001", "station_002"},
    };
    Line line {
        "line_000",
        "Line Name",
        {route0},
    };
    ok = true;
    ok &= nw.addStation(station0);
    ok &= nw.addStation(station1);
    ok &= nw.addStation(station2);
    BOOST_REQUIRE(ok);
    ok = nw.addLine(line);
    BOOST_REQUIRE(ok);

    unsigned int travelTime {0};

    // Get travel time before setting it.
    travelTime = nw.getAdjacentTravelTime(station0.id, station1.id);
    BOOST_CHECK_EQUAL(travelTime, 0);

    // Cannot set the travel time between non-adjacent stations.
    ok = nw.setTravelTime(station0.id, station2.id, 1);
    BOOST_CHECK(!ok);

    // Set the travel time between adjacent stations.
    ok = nw.setTravelTime(station0.id, station1.id, 2);
    BOOST_CHECK(ok);
    BOOST_CHECK_EQUAL(nw.getAdjacentTravelTime(station0.id, station1.id), 2);

    // Set travel time between adjacend stations, even if they appear in the
    // reverse order in the route.
    ok = nw.setTravelTime(station1.id, station0.id, 3);
    BOOST_CHECK(ok);
    BOOST_CHECK_EQUAL(nw.getAdjacentTravelTime(station1.id, station0.id), 3);
}

BOOST_AUTO_TEST_CASE(over_route)
{
    TransportNetwork nw {};
    bool ok {false};

    // Add a line with 3 routes.
    // route0: 0 ---> 1 ---> 2 ---> 3
    // route1: 3 ---> 1 ---> 2
    // route2: 3 ---> 1 ---> 0
    Station station0 {
        "station_000",
        "Station Name 0",
    };
    Station station1 {
        "station_001",
        "Station Name 1",
    };
    Station station2 {
        "station_002",
        "Station Name 2",
    };
    Station station3 {
        "station_003",
        "Station Name 3",
    };
    Route route0 {
        "route_000",
        "inbound",
        "line_000",
        "station_000",
        "station_003",
        {"station_000", "station_001", "station_002", "station_003"},
    };
    Route route1 {
        "route_001",
        "inbound",
        "line_000",
        "station_003",
        "station_002",
        {"station_003", "station_001", "station_002"},
    };
    Route route2 {
        "route_002",
        "inbound",
        "line_000",
        "station_003",
        "station_000",
        {"station_003", "station_001", "station_000"},
    };
    Line line {
        "line_000",
        "Line Name",
        {route0, route1, route2},
    };
    ok = true;
    ok &= nw.addStation(station0);
    ok &= nw.addStation(station1);
    ok &= nw.addStation(station2);
    ok &= nw.addStation(station3);
    BOOST_REQUIRE(ok);
    ok = nw.addLine(line);
    BOOST_REQUIRE(ok);

    // Set all travel times.
    ok = true;
    ok &= nw.setTravelTime(station0.id, station1.id, 1);
    ok &= nw.setTravelTime(station1.id, station2.id, 2);
    ok &= nw.setTravelTime(station2.id, station3.id, 3);
    ok &= nw.setTravelTime(station3.id, station1.id, 4);
    BOOST_REQUIRE(ok);

    // Check the cumulative travel times.
    unsigned int travelTime {0};
    // route0
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route0.id, station0.id, station1.id), 1
    );
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route0.id, station0.id, station2.id), 1 + 2
    );
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route0.id, station0.id, station3.id), 1 + 2 + 3
    );
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route0.id, station1.id, station3.id), 2 + 3
    );
    // route1
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route1.id, station3.id, station1.id), 4
    );
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route1.id, station3.id, station2.id), 4 + 2
    );
    // route2
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route2.id, station3.id, station1.id), 4
    );
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route2.id, station3.id, station0.id), 4 + 1
    );
    // Invalid routes
    // -- 3 -> 1 is possible, but only over route1 and route2.
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route0.id, station3.id, station1.id), 0
    );
    // -- 1 -> 0 is possible, but only over route3.
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route0.id, station1.id, station0.id), 0
    );
    BOOST_CHECK_EQUAL(
        nw.getTravelTime(line.id, route0.id, station1.id, station1.id), 0
    );
}

BOOST_AUTO_TEST_SUITE_END(); // TravelTime

BOOST_AUTO_TEST_SUITE_END(); // class_TransportNetwork

BOOST_AUTO_TEST_SUITE_END(); // websocket_client
