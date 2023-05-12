
#include <network_monitor/transport_network.hpp>

#include <nlohmann/json.hpp>

#include <cstddef>
#include <set>
#include <stdexcept>

using std::size_t;

namespace NetworkMonitor {

bool TransportNetwork::fromJson(nlohmann::json&& src) {
    m_nodeMp = {};

    const auto& stations = src.at("stations");
    for(const auto& stationJson : stations) {
        const Station station{
            .id = stationJson.at("station_id"),
            .name = stationJson.at("name"),
        };
        if(!addStation(station) )
            throw std::runtime_error("Unable to add station: " + station.id);
    }

    const auto& lines = src.at("lines");
    for(const auto& lineJson : lines) {
        Line line{
            .id = lineJson.at("line_id"),
            .name = lineJson.at("name"),
        };
        const auto& lineJsonRoutes = lineJson.at("routes");
        for(const auto& routeJson : lineJsonRoutes) {
            Route route {
                .id = routeJson.at("route_id"),
                .direction = routeJson.at("direction"),
                .startStationId = routeJson.at("start_station_id"),
                .endStationId = routeJson.at("end_station_id"),
            };
            const auto& routeStops = routeJson.at("route_stops");
            for(auto& station : routeStops) {
                route.stops.push_back(station);
            }
            line.routes.push_back(std::move(route) );
        }
        if(!addLine(line) )
            throw std::runtime_error("Unable to add line: " + line.id);
    }

    const auto travelTimes = src.at("travel_times");
    for(const auto& travelTime : travelTimes) {
        const Id stationA = travelTime.at("start_station_id");
        const Id stationB = travelTime.at("end_station_id");
        const unsigned int time = travelTime.at("travel_time");
        if(!setTravelTime(stationA, stationB, time) )
            return false;
    }
    return true;
}

bool TransportNetwork::addStation(const Station& station) {
    return m_nodeMp.insert({
        station.id,
        Node{
            .stationName = station.name,
            .passengerCount = 0,
            .outEdges = {},
        }
    }).second;
}

bool TransportNetwork::addLine(const Line& line) {
    for(const Route& route : line.routes) {
        if(route.stops.size() < 2)
            return false;
        for(const auto& stationId : route.stops) {
            if(!m_nodeMp.contains(stationId) )
                return false;
        }

        for(size_t i=0; i+1<route.stops.size(); ++i) {
            const Id& fromStationId = route.stops[i];
            Node& fromNode = m_nodeMp[fromStationId];
            const Id& toStationId = route.stops[i+1];

            Node& toNode = m_nodeMp[toStationId];

            auto& outEdges = fromNode.outEdges[&toNode];
            if(outEdges.routes.contains(route.id) )
                return false;
            outEdges.routes.insert(route.id);
            toNode.inEdges[&fromNode] = outEdges;
        }
    }
    return true;
}

bool TransportNetwork::recordPassengerEvent(const PassengerEvent& event) {
    if(!m_nodeMp.contains(event.stationId) )
        return false;

    auto& passengerCount = m_nodeMp[event.stationId].passengerCount;
    switch(event.type) {
    case PassengerEvent::Type::In:
        ++passengerCount;
        return true;
    case PassengerEvent::Type::Out:
        --passengerCount;
        return true;
    }
    return false;
}

long long TransportNetwork::getPassengerCount(const Id& station) const {
    if(!m_nodeMp.contains(station) )
        throw std::runtime_error("Station is not found in network: " + station);
    return m_nodeMp.at(station).passengerCount;
}

std::vector<Id> TransportNetwork::getRoutesServingStation(const Id& station) const {
    if(!m_nodeMp.contains(station) )
        return {};

    const auto& node = m_nodeMp.at(station);
    std::set<Id> routeSet;
    for(const auto& [_, edge] : node.outEdges) {
        for(const auto& route : edge.routes)
            routeSet.insert(route);
    }
    for(const auto& [_, edge] : node.inEdges) {
        for(const auto& route : edge.routes)
            routeSet.insert(route);
    }

    std::vector<Id> routeLst;
    routeLst.reserve(routeSet.size() );
    for(const auto& route : routeSet)
        routeLst.push_back(route);
    return routeLst;
}

bool TransportNetwork::setTravelTime(
        const Id& stationA,
        const Id& stationB,
        const unsigned int travelTime)
{
    if(!m_nodeMp.contains(stationA) || !m_nodeMp.contains(stationB) )
        return false;

    auto& nodeA = m_nodeMp[stationA];
    auto& nodeB = m_nodeMp[stationB];

    bool adjacent = false;
    if(nodeA.outEdges.contains(&nodeB) ) { // A->B
        nodeA.outEdges.at(&nodeB).travelTime = travelTime;
        nodeB.inEdges.at(&nodeA).travelTime = travelTime;
        adjacent = true;
    }
    if(nodeA.inEdges.contains(&nodeB) ) { // B->A
        nodeA.inEdges.at(&nodeB).travelTime = travelTime;
        nodeB.outEdges.at(&nodeA).travelTime = travelTime;
        adjacent = true;
    }
    if(!adjacent)
        return false;

    return true;
}

unsigned int TransportNetwork::getAdjacentTravelTime(const Id& stationA, const Id& stationB) const {
    if(!m_nodeMp.contains(stationA) || !m_nodeMp.contains(stationB) )
        return 0;

    const auto& nodeA = m_nodeMp.at(stationA);
    const auto& nodeB = m_nodeMp.at(stationB);
    if(nodeA.outEdges.contains(&nodeB) ) { // A->B
        return nodeA.outEdges.at(&nodeB).travelTime;
    } else if(nodeA.inEdges.contains(&nodeB) ) { // B->A
        return nodeA.inEdges.at(&nodeB).travelTime;
    } else { //not adjacent
        return 0;
    }
}

//TODO: Fix bugs
unsigned int TransportNetwork::getTravelTime(
        const Id&,
        const Id& route,
        const Id& stationA,
        const Id& stationB) const
{
    if(!m_nodeMp.contains(stationA) || !m_nodeMp.contains(stationB) )
        return 0;

    const auto* from = &m_nodeMp.at(stationA);
    const auto& to = m_nodeMp.at(stationB);

    int travelTime = 0;
    while(from != &to) {
        bool foundRoute = false;
        for(const auto& [next, edge]: from->outEdges) {
            if(edge.routes.contains(route) ) {
                travelTime += edge.travelTime;
                from = next;
                foundRoute = true;
                break;
            }
        }
        if(!foundRoute)
            return 0;
    }
    return travelTime;
}

} //NetworkMonitor
