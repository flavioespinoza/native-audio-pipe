#ifndef NAP_AUDIOGRAPH_H
#define NAP_AUDIOGRAPH_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nap {

class IAudioNode;
class ConnectionManager;
class ExecutionSorter;
class FeedbackLoopDetector;

/**
 * @brief Central audio processing graph that manages nodes and their connections.
 *
 * AudioGraph is the main container for audio processing nodes. It handles node
 * registration, connection management, topological sorting, and coordinated processing.
 */
class AudioGraph {
public:
    AudioGraph();
    ~AudioGraph();

    AudioGraph(const AudioGraph&) = delete;
    AudioGraph& operator=(const AudioGraph&) = delete;
    AudioGraph(AudioGraph&&) noexcept;
    AudioGraph& operator=(AudioGraph&&) noexcept;

    /**
     * @brief Add a node to the graph.
     * @param node The node to add
     * @return True if added successfully, false if node already exists
     */
    bool addNode(std::shared_ptr<IAudioNode> node);

    /**
     * @brief Remove a node from the graph.
     * @param nodeId The ID of the node to remove
     * @return True if removed successfully, false if node not found
     */
    bool removeNode(const std::string& nodeId);

    /**
     * @brief Get a node by its ID.
     * @param nodeId The ID of the node to retrieve
     * @return Shared pointer to the node, or nullptr if not found
     */
    std::shared_ptr<IAudioNode> getNode(const std::string& nodeId) const;

    /**
     * @brief Connect two nodes.
     * @param sourceNodeId The ID of the source node
     * @param sourceChannel The output channel of the source
     * @param destNodeId The ID of the destination node
     * @param destChannel The input channel of the destination
     * @return True if connection was successful
     */
    bool connect(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                 const std::string& destNodeId, std::uint32_t destChannel);

    /**
     * @brief Disconnect two nodes.
     * @param sourceNodeId The ID of the source node
     * @param sourceChannel The output channel of the source
     * @param destNodeId The ID of the destination node
     * @param destChannel The input channel of the destination
     * @return True if disconnection was successful
     */
    bool disconnect(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                    const std::string& destNodeId, std::uint32_t destChannel);

    /**
     * @brief Process one block of audio through the entire graph.
     * @param numFrames Number of frames to process
     */
    void processBlock(std::uint32_t numFrames);

    /**
     * @brief Prepare all nodes for processing.
     * @param sampleRate The sample rate
     * @param blockSize The maximum block size
     */
    void prepare(double sampleRate, std::uint32_t blockSize);

    /**
     * @brief Reset all nodes in the graph.
     */
    void reset();

    /**
     * @brief Get the number of nodes in the graph.
     * @return Node count
     */
    std::size_t getNodeCount() const;

    /**
     * @brief Get all node IDs in the graph.
     * @return Vector of node IDs
     */
    std::vector<std::string> getAllNodeIds() const;

    /**
     * @brief Rebuild the processing order after topology changes.
     */
    void rebuildProcessingOrder();

    /**
     * @brief Check if the current graph configuration is valid.
     * @return True if valid, false if there are issues
     */
    bool isValid() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_AUDIOGRAPH_H
