#include "ScopeNode.h"
#include <algorithm>
#include <string>

namespace nap {

class ScopeNode::Impl {
public:
    std::string nodeId;
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    std::size_t writeIndex = 0;
    std::size_t bufferSize = 1024;
    float triggerLevel = 0.0f;
    bool triggerEnabled = false;
    bool triggered = false;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

ScopeNode::ScopeNode()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "ScopeNode_" + std::to_string(++instanceCounter);
    m_impl->bufferL.resize(m_impl->bufferSize, 0.0f);
    m_impl->bufferR.resize(m_impl->bufferSize, 0.0f);
}

ScopeNode::~ScopeNode() = default;

ScopeNode::ScopeNode(ScopeNode&&) noexcept = default;
ScopeNode& ScopeNode::operator=(ScopeNode&&) noexcept = default;

void ScopeNode::process(const float* inputBuffer, float* outputBuffer,
                         std::uint32_t numFrames, std::uint32_t numChannels)
{
    std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);

    if (m_impl->bypassed) return;

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float sampleL = inputBuffer[i * numChannels];
        float sampleR = numChannels > 1 ? inputBuffer[i * numChannels + 1] : sampleL;

        if (m_impl->triggerEnabled && !m_impl->triggered) {
            if (sampleL >= m_impl->triggerLevel) {
                m_impl->triggered = true;
                m_impl->writeIndex = 0;
            }
        }

        if (!m_impl->triggerEnabled || m_impl->triggered) {
            m_impl->bufferL[m_impl->writeIndex] = sampleL;
            m_impl->bufferR[m_impl->writeIndex] = sampleR;
            m_impl->writeIndex = (m_impl->writeIndex + 1) % m_impl->bufferSize;

            if (m_impl->triggerEnabled && m_impl->writeIndex == 0) {
                m_impl->triggered = false;
            }
        }
    }
}

void ScopeNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void ScopeNode::reset()
{
    std::fill(m_impl->bufferL.begin(), m_impl->bufferL.end(), 0.0f);
    std::fill(m_impl->bufferR.begin(), m_impl->bufferR.end(), 0.0f);
    m_impl->writeIndex = 0;
    m_impl->triggered = false;
}

std::string ScopeNode::getNodeId() const { return m_impl->nodeId; }
std::string ScopeNode::getTypeName() const { return "ScopeNode"; }
std::uint32_t ScopeNode::getNumInputChannels() const { return 2; }
std::uint32_t ScopeNode::getNumOutputChannels() const { return 2; }
bool ScopeNode::isBypassed() const { return m_impl->bypassed; }
void ScopeNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void ScopeNode::setBufferSize(std::size_t samples)
{
    m_impl->bufferSize = samples;
    m_impl->bufferL.resize(samples, 0.0f);
    m_impl->bufferR.resize(samples, 0.0f);
    m_impl->writeIndex = 0;
}

std::size_t ScopeNode::getBufferSize() const { return m_impl->bufferSize; }

std::vector<float> ScopeNode::getWaveformData(std::uint32_t channel) const
{
    const auto& buffer = channel == 0 ? m_impl->bufferL : m_impl->bufferR;
    std::vector<float> result(m_impl->bufferSize);

    for (std::size_t i = 0; i < m_impl->bufferSize; ++i) {
        result[i] = buffer[(m_impl->writeIndex + i) % m_impl->bufferSize];
    }

    return result;
}

void ScopeNode::setTriggerLevel(float level) { m_impl->triggerLevel = level; }
float ScopeNode::getTriggerLevel() const { return m_impl->triggerLevel; }
void ScopeNode::setTriggerEnabled(bool enabled) { m_impl->triggerEnabled = enabled; }
bool ScopeNode::isTriggerEnabled() const { return m_impl->triggerEnabled; }

} // namespace nap
