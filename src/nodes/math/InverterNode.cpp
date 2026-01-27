#include "InverterNode.h"
#include <algorithm>
#include <string>

namespace nap {

class InverterNode::Impl {
public:
    std::string nodeId;
    bool invertLeft = true;
    bool invertRight = true;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

InverterNode::InverterNode()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "InverterNode_" + std::to_string(++instanceCounter);
}

InverterNode::~InverterNode() = default;

InverterNode::InverterNode(InverterNode&&) noexcept = default;
InverterNode& InverterNode::operator=(InverterNode&&) noexcept = default;

void InverterNode::process(const float* inputBuffer, float* outputBuffer,
                            std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    if (numChannels >= 2) {
        for (std::uint32_t i = 0; i < numFrames; ++i) {
            outputBuffer[i * 2] = m_impl->invertLeft ? -inputBuffer[i * 2] : inputBuffer[i * 2];
            outputBuffer[i * 2 + 1] = m_impl->invertRight ? -inputBuffer[i * 2 + 1] : inputBuffer[i * 2 + 1];
        }
    } else {
        for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
            outputBuffer[i] = m_impl->invertLeft ? -inputBuffer[i] : inputBuffer[i];
        }
    }
}

void InverterNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void InverterNode::reset()
{
    // No state to reset
}

std::string InverterNode::getNodeId() const { return m_impl->nodeId; }
std::string InverterNode::getTypeName() const { return "InverterNode"; }
std::uint32_t InverterNode::getNumInputChannels() const { return 2; }
std::uint32_t InverterNode::getNumOutputChannels() const { return 2; }
bool InverterNode::isBypassed() const { return m_impl->bypassed; }
void InverterNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void InverterNode::setInvertLeft(bool invert) { m_impl->invertLeft = invert; }
bool InverterNode::getInvertLeft() const { return m_impl->invertLeft; }
void InverterNode::setInvertRight(bool invert) { m_impl->invertRight = invert; }
bool InverterNode::getInvertRight() const { return m_impl->invertRight; }
void InverterNode::setInvertBoth(bool invert)
{
    m_impl->invertLeft = invert;
    m_impl->invertRight = invert;
}

} // namespace nap
