#include "ExecutionSorter.h"
#include "ConnectionManager.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace nap {

class ExecutionSorter::Impl {
public:
    // Helper to build adjacency lists
    std::unordered_map<std::string, std::vector<std::string>> buildAdjacencyList(
        const std::vector<std::string>& nodeIds,
        const ConnectionManager& connectionManager) const
    {
        std::unordered_map<std::string, std::vector<std::string>> adjList;
        for (const auto& nodeId : nodeIds) {
            adjList[nodeId] = {};
        }

        for (const auto& conn : connectionManager.getAllConnections()) {
            adjList[conn.sourceNodeId].push_back(conn.destNodeId);
        }

        return adjList;
    }

    std::unordered_map<std::string, std::uint32_t> computeInDegrees(
        const std::vector<std::string>& nodeIds,
        const ConnectionManager& connectionManager) const
    {
        std::unordered_map<std::string, std::uint32_t> inDegree;
        for (const auto& nodeId : nodeIds) {
            inDegree[nodeId] = 0;
        }

        for (const auto& conn : connectionManager.getAllConnections()) {
            inDegree[conn.destNodeId]++;
        }

        return inDegree;
    }
};

ExecutionSorter::ExecutionSorter()
    : m_impl(std::make_unique<Impl>())
{
}

ExecutionSorter::~ExecutionSorter() = default;

ExecutionSorter::ExecutionSorter(ExecutionSorter&&) noexcept = default;
ExecutionSorter& ExecutionSorter::operator=(ExecutionSorter&&) noexcept = default;

std::vector<std::string> ExecutionSorter::computeExecutionOrder(
    const std::vector<std::string>& nodeIds,
    const ConnectionManager& connectionManager) const
{
    auto adjList = m_impl->buildAdjacencyList(nodeIds, connectionManager);
    auto inDegree = m_impl->computeInDegrees(nodeIds, connectionManager);

    std::queue<std::string> queue;
    for (const auto& [nodeId, degree] : inDegree) {
        if (degree == 0) {
            queue.push(nodeId);
        }
    }

    std::vector<std::string> result;
    result.reserve(nodeIds.size());

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        result.push_back(current);

        for (const auto& neighbor : adjList[current]) {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0) {
                queue.push(neighbor);
            }
        }
    }

    return result;
}

std::vector<std::vector<std::string>> ExecutionSorter::computeParallelGroups(
    const std::vector<std::string>& nodeIds,
    const ConnectionManager& connectionManager) const
{
    auto adjList = m_impl->buildAdjacencyList(nodeIds, connectionManager);
    auto inDegree = m_impl->computeInDegrees(nodeIds, connectionManager);

    std::vector<std::vector<std::string>> groups;
    std::unordered_set<std::string> processed;

    while (processed.size() < nodeIds.size()) {
        std::vector<std::string> currentGroup;

        for (const auto& nodeId : nodeIds) {
            if (processed.find(nodeId) == processed.end() && inDegree[nodeId] == 0) {
                currentGroup.push_back(nodeId);
            }
        }

        if (currentGroup.empty()) {
            break; // Cycle detected
        }

        for (const auto& nodeId : currentGroup) {
            processed.insert(nodeId);
            for (const auto& neighbor : adjList[nodeId]) {
                inDegree[neighbor]--;
            }
        }

        groups.push_back(std::move(currentGroup));
    }

    return groups;
}

bool ExecutionSorter::hasValidOrder(const std::vector<std::string>& nodeIds,
                                     const ConnectionManager& connectionManager) const
{
    auto sorted = computeExecutionOrder(nodeIds, connectionManager);
    return sorted.size() == nodeIds.size();
}

std::uint32_t ExecutionSorter::getNodeDepth(const std::string& nodeId,
                                             const ConnectionManager& connectionManager) const
{
    auto connections = connectionManager.getConnectionsTo(nodeId);
    if (connections.empty()) {
        return 0;
    }

    std::uint32_t maxDepth = 0;
    for (const auto& conn : connections) {
        std::uint32_t depth = getNodeDepth(conn.sourceNodeId, connectionManager);
        maxDepth = std::max(maxDepth, depth);
    }

    return maxDepth + 1;
}

} // namespace nap
