#include "AudioGraph.h"
#include "ConnectionManager.h"
#include "ExecutionSorter.h"
#include "FeedbackLoopDetector.h"
#include "../../api/IAudioNode.h"

namespace nap {

class AudioGraph::Impl {
public:
    std::unordered_map<std::string, std::shared_ptr<IAudioNode>> nodes;
    std::unique_ptr<ConnectionManager> connectionManager;
    std::unique_ptr<ExecutionSorter> executionSorter;
    std::unique_ptr<FeedbackLoopDetector> feedbackDetector;
    std::vector<std::string> processingOrder;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool needsRebuild = true;
};

AudioGraph::AudioGraph()
    : m_impl(std::make_unique<Impl>())
{
    m_impl->connectionManager = std::make_unique<ConnectionManager>();
    m_impl->executionSorter = std::make_unique<ExecutionSorter>();
    m_impl->feedbackDetector = std::make_unique<FeedbackLoopDetector>();
}

AudioGraph::~AudioGraph() = default;

AudioGraph::AudioGraph(AudioGraph&&) noexcept = default;
AudioGraph& AudioGraph::operator=(AudioGraph&&) noexcept = default;

bool AudioGraph::addNode(std::shared_ptr<IAudioNode> node)
{
    if (!node) {
        return false;
    }

    const auto& nodeId = node->getNodeId();
    if (m_impl->nodes.find(nodeId) != m_impl->nodes.end()) {
        return false;
    }

    m_impl->nodes[nodeId] = std::move(node);
    m_impl->needsRebuild = true;
    return true;
}

bool AudioGraph::removeNode(const std::string& nodeId)
{
    auto it = m_impl->nodes.find(nodeId);
    if (it == m_impl->nodes.end()) {
        return false;
    }

    m_impl->connectionManager->removeAllConnectionsForNode(nodeId);
    m_impl->nodes.erase(it);
    m_impl->needsRebuild = true;
    return true;
}

std::shared_ptr<IAudioNode> AudioGraph::getNode(const std::string& nodeId) const
{
    auto it = m_impl->nodes.find(nodeId);
    if (it != m_impl->nodes.end()) {
        return it->second;
    }
    return nullptr;
}

bool AudioGraph::connect(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                         const std::string& destNodeId, std::uint32_t destChannel)
{
    if (m_impl->nodes.find(sourceNodeId) == m_impl->nodes.end() ||
        m_impl->nodes.find(destNodeId) == m_impl->nodes.end()) {
        return false;
    }

    bool result = m_impl->connectionManager->addConnection(
        sourceNodeId, sourceChannel, destNodeId, destChannel);

    if (result) {
        m_impl->needsRebuild = true;
    }

    return result;
}

bool AudioGraph::disconnect(const std::string& sourceNodeId, std::uint32_t sourceChannel,
                            const std::string& destNodeId, std::uint32_t destChannel)
{
    bool result = m_impl->connectionManager->removeConnection(
        sourceNodeId, sourceChannel, destNodeId, destChannel);

    if (result) {
        m_impl->needsRebuild = true;
    }

    return result;
}

void AudioGraph::processBlock(std::uint32_t numFrames)
{
    if (m_impl->needsRebuild) {
        rebuildProcessingOrder();
    }

    // TODO: Implement block processing with proper buffer management
}

void AudioGraph::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;

    for (auto& [id, node] : m_impl->nodes) {
        node->prepare(sampleRate, blockSize);
    }
}

void AudioGraph::reset()
{
    for (auto& [id, node] : m_impl->nodes) {
        node->reset();
    }
}

std::size_t AudioGraph::getNodeCount() const
{
    return m_impl->nodes.size();
}

std::vector<std::string> AudioGraph::getAllNodeIds() const
{
    std::vector<std::string> ids;
    ids.reserve(m_impl->nodes.size());
    for (const auto& [id, node] : m_impl->nodes) {
        ids.push_back(id);
    }
    return ids;
}

void AudioGraph::rebuildProcessingOrder()
{
    // TODO: Use ExecutionSorter to compute topological order
    m_impl->needsRebuild = false;
}

bool AudioGraph::isValid() const
{
    // TODO: Check for cycles using FeedbackLoopDetector
    return true;
}

} // namespace nap
