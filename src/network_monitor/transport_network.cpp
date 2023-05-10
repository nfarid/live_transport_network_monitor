
#include <network_monitor/transport_network.hpp>

namespace NetworkMonitor {

bool TransportNetwork::addStation(const Station& station) {
    //TODO: Implement
    return false;
}

bool TransportNetwork::addLine(const Line& line) {
    //TODO: Implement
    return false;
}

bool TransportNetwork::recordPassengerEvent(const PassengerEvent& event) {
    //TODO: Implement
    return false;
}

long long TransportNetwork::getPassengerCount(const Id& station) const {
    //TODO: Implement
    return 0;
}

std::vector<Id> TransportNetwork::getRoutesServingStation(const Id& station) const {
    //TODO: Implement
    return {};
}

bool TransportNetwork::setTravelTime(
        const Id& stationA,
        const Id& stationB,
        const unsigned int travelTime)
{
    //TODO: Implement
    return false;
}

unsigned int TransportNetwork::getAdjacentTravelTime(const Id& stationA, const Id& stationB) const {
    //TODO: Implement
    return 0;
}

unsigned int TransportNetwork::getTravelTime(
        const Id& line,
        const Id& route,
        const Id& stationA,
        const Id& stationB) const
{
    //TODO: Implement
    return 0;
}

} //NetworkMonitor
