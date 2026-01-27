#include "SimpleDelay.h"
#include <algorithm>
#include <string>
#include <vector>

namespace nap {

class SimpleDelay::Impl {
public:
    std::string nodeId;
    std::vector<float> delayBuffer;
    std::size_t writeIndex = 0;
    float delayTimeMs = 250.0f;
    float maxDelayTimeMs = 2000.0f;
    float feedback = 0.3f;
    float mix = 0.5f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    std::size_t delaySamples = 0;
    bool bypassed = false;

    void updateDelaySamples() {
        delaySamples = static_cast<std::size_t>(delayTimeMs * sampleRate / 1000.0);
    }
};

SimpleDelay::SimpleDelay()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "SimpleDelay_" + std::to_string(++instanceCounter);
}

SimpleDelay::~SimpleDelay() = default;

SimpleDelay::SimpleDelay(SimpleDelay&&) noexcept = default;
SimpleDelay& SimpleDelay::operator=(SimpleDelay&&) noexcept = default;

void SimpleDelay::process(const float* inputBuffer, float* outputBuffer,
                           std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
        std::size_t readIndex = (m_impl->writeIndex + m_impl->delayBuffer.size() - m_impl->delaySamples)
                                % m_impl->delayBuffer.size();

        float delayedSample = m_impl->delayBuffer[readIndex];
        float inputSample = inputBuffer[i];

        m_impl->delayBuffer[m_impl->writeIndex] = inputSample + delayedSample * m_impl->feedback;
        m_impl->writeIndex = (m_impl->writeIndex + 1) % m_impl->delayBuffer.size();

        outputBuffer[i] = inputSample * (1.0f - m_impl->mix) + delayedSample * m_impl->mix;
    }
}

void SimpleDelay::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    std::size_t maxSamples = static_cast<std::size_t>(m_impl->maxDelayTimeMs * sampleRate / 1000.0);
    m_impl->delayBuffer.resize(maxSamples, 0.0f);
    m_impl->updateDelaySamples();
}

void SimpleDelay::reset()
{
    std::fill(m_impl->delayBuffer.begin(), m_impl->delayBuffer.end(), 0.0f);
    m_impl->writeIndex = 0;
}

std::string SimpleDelay::getNodeId() const { return m_impl->nodeId; }
std::string SimpleDelay::getTypeName() const { return "SimpleDelay"; }
std::uint32_t SimpleDelay::getNumInputChannels() const { return 2; }
std::uint32_t SimpleDelay::getNumOutputChannels() const { return 2; }
bool SimpleDelay::isBypassed() const { return m_impl->bypassed; }
void SimpleDelay::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void SimpleDelay::setDelayTime(float milliseconds)
{
    m_impl->delayTimeMs = std::min(milliseconds, m_impl->maxDelayTimeMs);
    m_impl->updateDelaySamples();
}

float SimpleDelay::getDelayTime() const { return m_impl->delayTimeMs; }
void SimpleDelay::setFeedback(float feedback) { m_impl->feedback = std::max(0.0f, std::min(0.99f, feedback)); }
float SimpleDelay::getFeedback() const { return m_impl->feedback; }
void SimpleDelay::setMix(float mix) { m_impl->mix = std::max(0.0f, std::min(1.0f, mix)); }
float SimpleDelay::getMix() const { return m_impl->mix; }
void SimpleDelay::setMaxDelayTime(float milliseconds) { m_impl->maxDelayTimeMs = milliseconds; }
float SimpleDelay::getMaxDelayTime() const { return m_impl->maxDelayTimeMs; }

} // namespace nap
