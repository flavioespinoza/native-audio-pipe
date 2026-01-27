#include "SplitterNode.h"
#include <algorithm>
#include <string>
#include <vector>

namespace nap {

class SplitterNode::Impl {
public:
    explicit Impl(std::uint32_t numOutputs)
        : numOutputs(numOutputs)
        , outputGains(numOutputs, 1.0f)
        , outputMuted(numOutputs, false)
    {
    }

    std::string nodeId;
    std::uint32_t numOutputs;
    std::vector<float> outputGains;
    std::vector<bool> outputMuted;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

SplitterNode::SplitterNode(std::uint32_t numOutputs)
    : m_impl(std::make_unique<Impl>(numOutputs))
{
    static int instanceCounter = 0;
    m_impl->nodeId = "SplitterNode_" + std::to_string(++instanceCounter);
}

SplitterNode::~SplitterNode() = default;

SplitterNode::SplitterNode(SplitterNode&&) noexcept = default;
SplitterNode& SplitterNode::operator=(SplitterNode&&) noexcept = default;

void SplitterNode::process(const float* inputBuffer, float* outputBuffer,
                            std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    // Copy input to all outputs
    for (std::uint32_t out = 0; out < m_impl->numOutputs; ++out) {
        if (!m_impl->outputMuted[out]) {
            float gain = m_impl->outputGains[out];
            for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
                outputBuffer[out * numFrames * numChannels + i] = inputBuffer[i] * gain;
            }
        }
    }
}

void SplitterNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void SplitterNode::reset()
{
    std::fill(m_impl->outputGains.begin(), m_impl->outputGains.end(), 1.0f);
    std::fill(m_impl->outputMuted.begin(), m_impl->outputMuted.end(), false);
}

std::string SplitterNode::getNodeId() const { return m_impl->nodeId; }
std::string SplitterNode::getTypeName() const { return "SplitterNode"; }
std::uint32_t SplitterNode::getNumInputChannels() const { return 2; }
std::uint32_t SplitterNode::getNumOutputChannels() const { return m_impl->numOutputs * 2; }
bool SplitterNode::isBypassed() const { return m_impl->bypassed; }
void SplitterNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

std::uint32_t SplitterNode::getNumOutputs() const { return m_impl->numOutputs; }

void SplitterNode::setOutputGain(std::uint32_t outputIndex, float gain)
{
    if (outputIndex < m_impl->numOutputs) {
        m_impl->outputGains[outputIndex] = gain;
    }
}

float SplitterNode::getOutputGain(std::uint32_t outputIndex) const
{
    return outputIndex < m_impl->numOutputs ? m_impl->outputGains[outputIndex] : 0.0f;
}

void SplitterNode::muteOutput(std::uint32_t outputIndex, bool mute)
{
    if (outputIndex < m_impl->numOutputs) {
        m_impl->outputMuted[outputIndex] = mute;
    }
}

bool SplitterNode::isOutputMuted(std::uint32_t outputIndex) const
{
    return outputIndex < m_impl->numOutputs ? m_impl->outputMuted[outputIndex] : true;
}

} // namespace nap
