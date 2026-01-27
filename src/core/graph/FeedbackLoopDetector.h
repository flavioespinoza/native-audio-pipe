#ifndef NAP_FEEDBACKLOOPDETECTOR_H
#define NAP_FEEDBACKLOOPDETECTOR_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nap {

class ConnectionManager;

/**
 * @brief Represents a detected cycle in the audio graph.
 */
struct CycleInfo {
    std::vector<std::string> nodeIds;
    std::string description;
};

/**
 * @brief Detects feedback loops and cycles in the audio graph.
 *
 * FeedbackLoopDetector analyzes the graph structure to identify cycles
 * that would prevent valid topological ordering and could cause
 * processing issues.
 */
class FeedbackLoopDetector {
public:
    FeedbackLoopDetector();
    ~FeedbackLoopDetector();

    FeedbackLoopDetector(const FeedbackLoopDetector&) = delete;
    FeedbackLoopDetector& operator=(const FeedbackLoopDetector&) = delete;
    FeedbackLoopDetector(FeedbackLoopDetector&&) noexcept;
    FeedbackLoopDetector& operator=(FeedbackLoopDetector&&) noexcept;

    /**
     * @brief Check if the graph contains any cycles.
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The connection manager containing graph edges
     * @return True if cycles are detected, false otherwise
     */
    bool hasCycle(const std::vector<std::string>& nodeIds,
                  const ConnectionManager& connectionManager) const;

    /**
     * @brief Find all cycles in the graph.
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The connection manager containing graph edges
     * @return Vector of CycleInfo structures describing each cycle
     */
    std::vector<CycleInfo> findAllCycles(
        const std::vector<std::string>& nodeIds,
        const ConnectionManager& connectionManager) const;

    /**
     * @brief Check if adding a connection would create a cycle.
     * @param sourceNodeId The source node of the proposed connection
     * @param destNodeId The destination node of the proposed connection
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The current connection manager
     * @return True if adding the connection would create a cycle
     */
    bool wouldCreateCycle(const std::string& sourceNodeId,
                          const std::string& destNodeId,
                          const std::vector<std::string>& nodeIds,
                          const ConnectionManager& connectionManager) const;

    /**
     * @brief Get nodes involved in cycles (cycle participants).
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The connection manager containing graph edges
     * @return Vector of node IDs that participate in at least one cycle
     */
    std::vector<std::string> getNodesInCycles(
        const std::vector<std::string>& nodeIds,
        const ConnectionManager& connectionManager) const;

    /**
     * @brief Suggest connections to remove to break all cycles.
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The connection manager containing graph edges
     * @return Vector of connection indices to remove
     */
    std::vector<std::pair<std::string, std::string>> suggestCycleBreaks(
        const std::vector<std::string>& nodeIds,
        const ConnectionManager& connectionManager) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_FEEDBACKLOOPDETECTOR_H
