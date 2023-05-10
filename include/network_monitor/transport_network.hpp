
#ifndef HPP_NETWORKMONITOR_TRANSPORTNETWORK_
#define HPP_NETWORKMONITOR_TRANSPORTNETWORK_

#include <string>
#include <vector>

namespace NetworkMonitor {

/*! \brief A station, line or route ID.
 */
using Id = std::string;

/*! \brief Network station
 *
 *  \note A station struct is well formed if:
 *          - `id` is unique acroos all stations in the network.
 */
struct Station {
    Id id{};
    std::string name{};

    bool operator==(const Station& other) const {
        return id == other.id;
    }
};

/*! \brief Network route
 *
 *  Each underground line has one or more routes.
 *  A route represents a single possible journey across a set of stops in a specified direction.
 *
 *  There may or may not be a corresponding route in the opposite direction of travel.
 *
 *  A Route struct is well-formed if:
 *      -`id` is unique across all lines and their routes in the network.
 *      - The `lineId` line exists and has this route among its routes.
 *      - `stops` has at least 2 stops.
 *      - `startStationId` is the first stop in `stops`.
 *      - `endStationId` is the last stop in `stops`.
 *      - Every `stationId` station in `stops` exists.
 *      - Every stop in `stops` appears only once.
 */
struct Route {
    Id id{};
    std::string direction{};
    Id lineId{};
    Id startStationId{};
    Id endStationId{};
    std::vector<Id> stops{};

    bool operator==(const Route& other) const {
        return id == other.id;
    }
};

/*! \brief Network line
 *
 *  A line is a collection of routes serving multiple stations.
 *
 *  A line struct is well-formed if:
 *      - `id` is unique across all lines in the network.
 *      - `routes` has at least 1 route.
 *      - Every route in `routes` is well-formed.
 *      - Every route in `routes` has a lineId` that is equal to this line `id`.
 */
struct Line {
    Id id{};
    std::string name{};
    std::vector<Route> routes{};

    bool operator==(const Line& other) const {
        return id == other.id;
    }
};

/*! \brief Passenger event
 */
struct PassengerEvent {
    enum class Type : unsigned {
        In,
        Out,
    };

    Id stationId{};
    Type type{};
};

/*! \brief Underground network representation
 */
class TransportNetwork {
public:
    /*! \brief Add a station to the network.
     *
     *  \param station - a well-formed station that's not in the network
     *
     *  \returns false if there was an error while adding the station to the network.
     */
    bool addStation(const Station& station);

    /*! \brief Add a line to the network.
     *
     *  \param line - a well-formed station that's not in the network.
     *              - all stations served by this line, must already be in the network.
     *
     * \returns false if there was an error while adding the station to the network.
     */
    bool addLine(const Line& line);

    /*! \brief Record a passenger event at a station.
     *
     *  \param event - must correspond to a station already in the network.
     *
     *  \returns false if the passenger event is not recognised.
     */
    bool recordPassengerEvent(const PassengerEvent& event);

    /*! \brief Get the number of passenger currently at a station.
     *
     *  \param station - must be a well-formed station already in the network.
     *
     *  \return can be positive or negative (if there's more exits events then enter events)
     *
     *  \throws std::runtime_error if the station is not in the network.
     */
    long long int getPassengerCount(const Id& station) const;

    /*! \brief Get list of routes serving a given station.
     *
     *  \param station - a well-formed station already in the network.
     *
     *  \returns An empty vector if the station has no routes, or if there was an error
     */
    std::vector<Id> getRoutesServingStation(const Id& station) const;

    /*! \brief Set the travel time between 2 adjacent stations.
     *
     *  \returns false if there was an error while setting the travel time between the stations.
     *
     *  \note The travel time is the same for all routes connecting the 2 stations directly.
     *  \note The 2 stations must be in the network and adjacent in a route.
     */
    bool setTravelTime(const Id& stationA, const Id& stationB, const unsigned int travelTime);

    /*! \brief Get the travel time between 2 adjacent stations.
     *
     *  \returns 0 if the function couldn't find the direct travel time.
     *
     *  \note The travel time is the same for all routes connecting the 2 stations directly.
     *  \note The 2 stations must be in the network and adjacent in a route.
     */
    unsigned int getAdjacentTravelTime(const Id& stationA, const Id& stationB) const;

    /*! \brief Get the total travel time between 2 stations on a specific route.
     *
     *  \returns - the cummulative sum of the travel time between all stations between A and B.
     *          0 if the function could not find the travel time, or if both stations are the same.
     *
     *  \note The travel time is the same for all routes connecting the 2 stations directly.
     *  \note The 2 stations must be in the network and served by the `route`.
     */
    unsigned int getTravelTime(
            const Id& line,
            const Id& route,
            const Id& stationA,
            const Id& stationB
    ) const;
};


} //namespace NetworkMonitor

#endif //HPP_NETWORKMONITOR_TRANSPORTNETWORK_
