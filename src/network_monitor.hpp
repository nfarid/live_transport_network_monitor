
#ifndef HPP_NETWORKMONITOR_
#define HPP_NETWORKMONITOR_

/*! \brief Configuration structure for the Live Transport Network Monitor
 *         process.
 */
struct NetworkMonitorConfig {
    // ...
};

/*! \brief Error codes for the Live Transport Network Monitor process.
 */
enum class NetworkMonitorError {
    kOk = 0,
    // ...
};

// ...

/*! \brief Live Transport Network Monitor
 *
 *  \tparam WsClient Type compatible with WebSocketClient.
 */
template <typename WsClient>
class NetworkMonitor {
public:

    // ...

    /*! \brief Setup the Live Transport Network Monitor.
     *
     *  This function only sets up the connection and performs error checks.
     *  It does not run the STOMP client.
     */
    explicit NetworkMonitorError Configure(const NetworkMonitorConfig& config)
    {
        // ...
    }

    /*! \brief Run the I/O context.
     *
     *  This function runs the I/O context in the current thread.
     */
    void Run()
    {
        // ...
    }

    // ...

};

#endif //HPP_NETWORKMONITOR_
