#include "FeedbackLoopDetector.h"
#include "ConnectionManager.h"
#include <algorithm>
#include <stack>
#include <unordered_map>
#include <unordered_set>

namespace nap {

class FeedbackLoopDetector::Impl {
public:
    enum class Color { White, Gray, Black };

    bool dfsHasCycle(
        const std::string& node,
        std::unordered_map<std::string, Color>& colors,
        const std::unordered_map<std::string, std::vector<std::string>>& adjList) const
    {
        colors[node] = Color::Gray;

        auto it = adjList.find(node);
        if (it != adjList.end()) {
            for (const auto& neighbor : it->second) {
                if (colors[neighbor] == Color::Gray) {
                    return true; // Back edge found, cycle exists
                }
                if (colors[neighbor] == Color::White) {
                    if (dfsHasCycle(neighbor, colors, adjList)) {
                        return true;
                    }
                }
            }
        }

        colors[node] = Color::Black;
        return false;
    }

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
};

FeedbackLoopDetector::FeedbackLoopDetector()
    : m_impl(std::make_unique<Impl>())
{
}

FeedbackLoopDetector::~FeedbackLoopDetector() = default;

FeedbackLoopDetector::FeedbackLoopDetector(FeedbackLoopDetector&&) noexcept = default;
FeedbackLoopDetector& FeedbackLoopDetector::operator=(FeedbackLoopDetector&&) noexcept = default;

bool FeedbackLoopDetector::hasCycle(const std::vector<std::string>& nodeIds,
                                     const ConnectionManager& connectionManager) const
{
    auto adjList = m_impl->buildAdjacencyList(nodeIds, connectionManager);
    std::unordered_map<std::string, Impl::Color> colors;

    for (const auto& nodeId : nodeIds) {
        colors[nodeId] = Impl::Color::White;
    }

    for (const auto& nodeId : nodeIds) {
        if (colors[nodeId] == Impl::Color::White) {
            if (m_impl->dfsHasCycle(nodeId, colors, adjList)) {
                return true;
            }
        }
    }

    return false;
}

std::vector<CycleInfo> FeedbackLoopDetector::findAllCycles(
    const std::vector<std::string>& nodeIds,
    const ConnectionManager& connectionManager) const
{
    std::vector<CycleInfo> cycles;
    auto adjList = m_impl->buildAdjacencyList(nodeIds, connectionManager);

    // TODO: Implement Johnson's algorithm for finding all cycles
    // For now, return empty if no cycle, or a placeholder if cycle exists
    if (hasCycle(nodeIds, connectionManager)) {
        CycleInfo info;
        info.description = "Cycle detected in graph";
        cycles.push_back(info);
    }

    return cycles;
}

bool FeedbackLoopDetector::wouldCreateCycle(const std::string& sourceNodeId,
                                             const std::string& destNodeId,
                                             const std::vector<std::string>& nodeIds,
                                             const ConnectionManager& connectionManager) const
{
    // Check if there's already a path from dest to source
    // If so, adding source->dest would create a cycle
    auto adjList = m_impl->buildAdjacencyList(nodeIds, connectionManager);

    std::unordered_set<std::string> visited;
    std::stack<std::string> stack;
    stack.push(destNodeId);

    while (!stack.empty()) {
        std::string current = stack.top();
        stack.pop();

        if (current == sourceNodeId) {
            return true; // Path exists from dest to source
        }

        if (visited.find(current) != visited.end()) {
            continue;
        }
        visited.insert(current);

        auto it = adjList.find(current);
        if (it != adjList.end()) {
            for (const auto& neighbor : it->second) {
                stack.push(neighbor);
            }
        }
    }

    return false;
}

std::vector<std::string> FeedbackLoopDetector::getNodesInCycles(
    const std::vector<std::string>& nodeIds,
    const ConnectionManager& connectionManager) const
{
    std::vector<std::string> result;
    auto cycles = findAllCycles(nodeIds, connectionManager);

    std::unordered_set<std::string> nodesInCycles;
    for (const auto& cycle : cycles) {
        for (const auto& nodeId : cycle.nodeIds) {
            nodesInCycles.insert(nodeId);
        }
    }

    result.assign(nodesInCycles.begin(), nodesInCycles.end());
    return result;
}

std::vector<std::pair<std::string, std::string>> FeedbackLoopDetector::suggestCycleBreaks(
    const std::vector<std::string>& nodeIds,
    const ConnectionManager& connectionManager) const
{
    std::vector<std::pair<std::string, std::string>> suggestions;

    // TODO: Implement minimum feedback arc set algorithm
    // For now, return empty suggestions

    return suggestions;
}

} // namespace nap
