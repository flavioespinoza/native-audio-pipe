#ifndef NAP_CONNECTIONMANAGER_H
#define NAP_CONNECTIONMANAGER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nap {

/**
 * @brief Represents a single connection between two nodes in the audio graph.
 */
struct Connection {
    std::string sourceNodeId;
    std::uint32_t sourceChannel;
    std::string destNodeId;
    std::uint32_t destChannel;

    bool operator==(const Connection& other) const;
};

/**
 * @brief Manages connections between audio nodes in the graph.
 *
 * ConnectionManager handles adding, removing, and querying connections
 * between audio nodes while maintaining connection integrity.
 */
class ConnectionManager {
public:
    ConnectionManager();
    ~ConnectionManager();

    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;
    ConnectionManager(ConnectionManager&&) noexcept;
    ConnectionManager& operator=(ConnectionManager&&) noexcept;

    /**
     * @brief Add a connection between two nodes.
     * @param sourceNodeId The ID of the source node
     * @param sourceChannel The output channel of the source
     * @param destNodeId The ID of the destination node
     * @param destChannel The input channel of the destination
     * @return True if connection was added successfully
     */
    bool addConnection(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                       const std::string& destNodeId, std::uint32_t destChannel);

    /**
     * @brief Remove a specific connection.
     * @param sourceNodeId The ID of the source node
     * @param sourceChannel The output channel of the source
     * @param destNodeId The ID of the destination node
     * @param destChannel The input channel of the destination
     * @return True if connection was removed successfully
     */
    bool removeConnection(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                          const std::string& destNodeId, std::uint32_t destChannel);

    /**
     * @brief Remove all connections involving a specific node.
     * @param nodeId The ID of the node
     */
    void removeAllConnectionsForNode(const std::string& nodeId);

    /**
     * @brief Get all connections from a specific node.
     * @param nodeId The ID of the source node
     * @return Vector of connections
     */
    std::vector<Connection> getConnectionsFrom(const std::string& nodeId) const;

    /**
     * @brief Get all connections to a specific node.
     * @param nodeId The ID of the destination node
     * @return Vector of connections
     */
    std::vector<Connection> getConnectionsTo(const std::string& nodeId) const;

    /**
     * @brief Get all connections in the graph.
     * @return Vector of all connections
     */
    std::vector<Connection> getAllConnections() const;

    /**
     * @brief Check if a specific connection exists.
     * @param sourceNodeId The ID of the source node
     * @param sourceChannel The output channel of the source
     * @param destNodeId The ID of the destination node
     * @param destChannel The input channel of the destination
     * @return True if the connection exists
     */
    bool hasConnection(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                       const std::string& destNodeId, std::uint32_t destChannel) const;

    /**
     * @brief Get the total number of connections.
     * @return Connection count
     */
    std::size_t getConnectionCount() const;

    /**
     * @brief Clear all connections.
     */
    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_CONNECTIONMANAGER_H
