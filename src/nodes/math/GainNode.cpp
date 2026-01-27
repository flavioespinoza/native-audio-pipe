#include "GainNode.h"
#include <cmath>
#include <string>

namespace nap {

class GainNode::Impl {
public:
    std::string nodeId;
    float gain = 1.0f;
    float targetGain = 1.0f;
    float smoothingCoeff = 0.995f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

GainNode::GainNode()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "GainNode_" + std::to_string(++instanceCounter);
}

GainNode::~GainNode() = default;

GainNode::GainNode(GainNode&&) noexcept = default;
GainNode& GainNode::operator=(GainNode&&) noexcept = default;

void GainNode::process(const float* inputBuffer, float* outputBuffer,
                        std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
        m_impl->gain = m_impl->gain * m_impl->smoothingCoeff +
                       m_impl->targetGain * (1.0f - m_impl->smoothingCoeff);
        outputBuffer[i] = inputBuffer[i] * m_impl->gain;
    }
}

void GainNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void GainNode::reset()
{
    m_impl->gain = m_impl->targetGain;
}

std::string GainNode::getNodeId() const { return m_impl->nodeId; }
std::string GainNode::getTypeName() const { return "GainNode"; }
std::uint32_t GainNode::getNumInputChannels() const { return 2; }
std::uint32_t GainNode::getNumOutputChannels() const { return 2; }
bool GainNode::isBypassed() const { return m_impl->bypassed; }
void GainNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void GainNode::setGain(float gainLinear) { m_impl->targetGain = gainLinear; }
float GainNode::getGain() const { return m_impl->targetGain; }
void GainNode::setGainDb(float gainDb) { m_impl->targetGain = std::pow(10.0f, gainDb / 20.0f); }
float GainNode::getGainDb() const { return 20.0f * std::log10(m_impl->targetGain); }

} // namespace nap
