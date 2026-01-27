#include "SummerNode.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class SummerNode::Impl {
public:
    explicit Impl(std::uint32_t numInputs)
        : numInputs(numInputs)
    {
    }

    std::string nodeId;
    std::uint32_t numInputs;
    bool normalize = false;
    bool clip = true;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

SummerNode::SummerNode(std::uint32_t numInputs)
    : m_impl(std::make_unique<Impl>(numInputs))
{
    static int instanceCounter = 0;
    m_impl->nodeId = "SummerNode_" + std::to_string(++instanceCounter);
}

SummerNode::~SummerNode() = default;

SummerNode::SummerNode(SummerNode&&) noexcept = default;
SummerNode& SummerNode::operator=(SummerNode&&) noexcept = default;

void SummerNode::process(const float* inputBuffer, float* outputBuffer,
                          std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    float normFactor = m_impl->normalize ? 1.0f / static_cast<float>(m_impl->numInputs) : 1.0f;

    // Sum all input samples (assuming interleaved input streams)
    std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);

    for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
        outputBuffer[i] = inputBuffer[i] * normFactor;

        if (m_impl->clip) {
            outputBuffer[i] = std::max(-1.0f, std::min(1.0f, outputBuffer[i]));
        }
    }
}

void SummerNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void SummerNode::reset()
{
    // No state to reset
}

std::string SummerNode::getNodeId() const { return m_impl->nodeId; }
std::string SummerNode::getTypeName() const { return "SummerNode"; }
std::uint32_t SummerNode::getNumInputChannels() const { return m_impl->numInputs * 2; }
std::uint32_t SummerNode::getNumOutputChannels() const { return 2; }
bool SummerNode::isBypassed() const { return m_impl->bypassed; }
void SummerNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

std::uint32_t SummerNode::getNumInputs() const { return m_impl->numInputs; }
void SummerNode::setNormalize(bool normalize) { m_impl->normalize = normalize; }
bool SummerNode::getNormalize() const { return m_impl->normalize; }
void SummerNode::setClip(bool clip) { m_impl->clip = clip; }
bool SummerNode::getClip() const { return m_impl->clip; }

} // namespace nap
