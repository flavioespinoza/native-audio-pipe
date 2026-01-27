#include "DCBlockerNode.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class DCBlockerNode::Impl {
public:
    std::string nodeId;
    float cutoffFrequency = 10.0f;
    float coefficient = 0.995f;
    float xPrevL = 0.0f, yPrevL = 0.0f;
    float xPrevR = 0.0f, yPrevR = 0.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    void updateCoefficient() {
        coefficient = 1.0f - (2.0f * 3.14159265f * cutoffFrequency / static_cast<float>(sampleRate));
        coefficient = std::max(0.9f, std::min(0.9999f, coefficient));
    }
};

DCBlockerNode::DCBlockerNode()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "DCBlockerNode_" + std::to_string(++instanceCounter);
}

DCBlockerNode::~DCBlockerNode() = default;

DCBlockerNode::DCBlockerNode(DCBlockerNode&&) noexcept = default;
DCBlockerNode& DCBlockerNode::operator=(DCBlockerNode&&) noexcept = default;

void DCBlockerNode::process(const float* inputBuffer, float* outputBuffer,
                             std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        // Left channel: y[n] = x[n] - x[n-1] + R * y[n-1]
        float xL = inputBuffer[i * numChannels];
        float yL = xL - m_impl->xPrevL + m_impl->coefficient * m_impl->yPrevL;
        m_impl->xPrevL = xL;
        m_impl->yPrevL = yL;
        outputBuffer[i * numChannels] = yL;

        if (numChannels > 1) {
            float xR = inputBuffer[i * numChannels + 1];
            float yR = xR - m_impl->xPrevR + m_impl->coefficient * m_impl->yPrevR;
            m_impl->xPrevR = xR;
            m_impl->yPrevR = yR;
            outputBuffer[i * numChannels + 1] = yR;
        }
    }
}

void DCBlockerNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->updateCoefficient();
}

void DCBlockerNode::reset()
{
    m_impl->xPrevL = m_impl->yPrevL = 0.0f;
    m_impl->xPrevR = m_impl->yPrevR = 0.0f;
}

std::string DCBlockerNode::getNodeId() const { return m_impl->nodeId; }
std::string DCBlockerNode::getTypeName() const { return "DCBlockerNode"; }
std::uint32_t DCBlockerNode::getNumInputChannels() const { return 2; }
std::uint32_t DCBlockerNode::getNumOutputChannels() const { return 2; }
bool DCBlockerNode::isBypassed() const { return m_impl->bypassed; }
void DCBlockerNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void DCBlockerNode::setCutoffFrequency(float frequencyHz)
{
    m_impl->cutoffFrequency = frequencyHz;
    m_impl->updateCoefficient();
}

float DCBlockerNode::getCutoffFrequency() const { return m_impl->cutoffFrequency; }

} // namespace nap
