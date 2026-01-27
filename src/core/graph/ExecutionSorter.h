#ifndef NAP_EXECUTIONSORTER_H
#define NAP_EXECUTIONSORTER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nap {

class ConnectionManager;

/**
 * @brief Computes topological sort order for audio graph execution.
 *
 * ExecutionSorter analyzes the graph structure and determines the optimal
 * order in which nodes should be processed to satisfy data dependencies.
 */
class ExecutionSorter {
public:
    ExecutionSorter();
    ~ExecutionSorter();

    ExecutionSorter(const ExecutionSorter&) = delete;
    ExecutionSorter& operator=(const ExecutionSorter&) = delete;
    ExecutionSorter(ExecutionSorter&&) noexcept;
    ExecutionSorter& operator=(ExecutionSorter&&) noexcept;

    /**
     * @brief Compute the execution order for the given nodes and connections.
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The connection manager containing graph edges
     * @return Sorted vector of node IDs in execution order
     */
    std::vector<std::string> computeExecutionOrder(
        const std::vector<std::string>& nodeIds,
        const ConnectionManager& connectionManager) const;

    /**
     * @brief Compute parallel execution groups.
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The connection manager containing graph edges
     * @return Vector of vectors, each inner vector contains nodes that can execute in parallel
     */
    std::vector<std::vector<std::string>> computeParallelGroups(
        const std::vector<std::string>& nodeIds,
        const ConnectionManager& connectionManager) const;

    /**
     * @brief Check if the graph has a valid topological order (no cycles).
     * @param nodeIds All node IDs in the graph
     * @param connectionManager The connection manager containing graph edges
     * @return True if a valid order exists, false if cycles are detected
     */
    bool hasValidOrder(const std::vector<std::string>& nodeIds,
                       const ConnectionManager& connectionManager) const;

    /**
     * @brief Get the depth of a node in the execution graph.
     * @param nodeId The node to query
     * @param connectionManager The connection manager containing graph edges
     * @return Depth level (0 for source nodes)
     */
    std::uint32_t getNodeDepth(const std::string& nodeId,
                               const ConnectionManager& connectionManager) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_EXECUTIONSORTER_H
