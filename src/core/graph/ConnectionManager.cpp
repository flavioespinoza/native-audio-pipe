#include "ConnectionManager.h"
#include <algorithm>
#include <unordered_set>

namespace nap {

bool Connection::operator==(const Connection& other) const
{
    return sourceNodeId == other.sourceNodeId &&
           sourceChannel == other.sourceChannel &&
           destNodeId == other.destNodeId &&
           destChannel == other.destChannel;
}

class ConnectionManager::Impl {
public:
    std::vector<Connection> connections;
};

ConnectionManager::ConnectionManager()
    : m_impl(std::make_unique<Impl>())
{
}

ConnectionManager::~ConnectionManager() = default;

ConnectionManager::ConnectionManager(ConnectionManager&&) noexcept = default;
ConnectionManager& ConnectionManager::operator=(ConnectionManager&&) noexcept = default;

bool ConnectionManager::addConnection(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                                       const std::string& destNodeId, std::uint32_t destChannel)
{
    if (hasConnection(sourceNodeId, sourceChannel, destNodeId, destChannel)) {
        return false;
    }

    m_impl->connections.push_back({sourceNodeId, sourceChannel, destNodeId, destChannel});
    return true;
}

bool ConnectionManager::removeConnection(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                                          const std::string& destNodeId, std::uint32_t destChannel)
{
    auto it = std::find_if(m_impl->connections.begin(), m_impl->connections.end(),
        [&](const Connection& conn) {
            return conn.sourceNodeId == sourceNodeId &&
                   conn.sourceChannel == sourceChannel &&
                   conn.destNodeId == destNodeId &&
                   conn.destChannel == destChannel;
        });

    if (it != m_impl->connections.end()) {
        m_impl->connections.erase(it);
        return true;
    }
    return false;
}

void ConnectionManager::removeAllConnectionsForNode(const std::string& nodeId)
{
    m_impl->connections.erase(
        std::remove_if(m_impl->connections.begin(), m_impl->connections.end(),
            [&nodeId](const Connection& conn) {
                return conn.sourceNodeId == nodeId || conn.destNodeId == nodeId;
            }),
        m_impl->connections.end());
}

std::vector<Connection> ConnectionManager::getConnectionsFrom(const std::string& nodeId) const
{
    std::vector<Connection> result;
    for (const auto& conn : m_impl->connections) {
        if (conn.sourceNodeId == nodeId) {
            result.push_back(conn);
        }
    }
    return result;
}

std::vector<Connection> ConnectionManager::getConnectionsTo(const std::string& nodeId) const
{
    std::vector<Connection> result;
    for (const auto& conn : m_impl->connections) {
        if (conn.destNodeId == nodeId) {
            result.push_back(conn);
        }
    }
    return result;
}

std::vector<Connection> ConnectionManager::getAllConnections() const
{
    return m_impl->connections;
}

bool ConnectionManager::hasConnection(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                                       const std::string& destNodeId, std::uint32_t destChannel) const
{
    return std::any_of(m_impl->connections.begin(), m_impl->connections.end(),
        [&](const Connection& conn) {
            return conn.sourceNodeId == sourceNodeId &&
                   conn.sourceChannel == sourceChannel &&
                   conn.destNodeId == destNodeId &&
                   conn.destChannel == destChannel;
        });
}

std::size_t ConnectionManager::getConnectionCount() const
{
    return m_impl->connections.size();
}

void ConnectionManager::clear()
{
    m_impl->connections.clear();
}

} // namespace nap
