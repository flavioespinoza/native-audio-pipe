#include "Flanger.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace nap {

class Flanger::Impl {
public:
    std::string nodeId;
    float rate = 0.25f;
    float depth = 0.5f;
    float feedback = 0.5f;
    float delayMs = 5.0f;
    float mix = 0.5f;
    float lfoPhase = 0.0f;
    float lfoIncrement = 0.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    std::size_t writeIndex = 0;
    float feedbackSampleL = 0.0f;
    float feedbackSampleR = 0.0f;

    void updateLfoIncrement() {
        lfoIncrement = rate / static_cast<float>(sampleRate);
    }
};

Flanger::Flanger()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "Flanger_" + std::to_string(++instanceCounter);
}

Flanger::~Flanger() = default;

Flanger::Flanger(Flanger&&) noexcept = default;
Flanger& Flanger::operator=(Flanger&&) noexcept = default;

void Flanger::process(const float* inputBuffer, float* outputBuffer,
                       std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float lfo = (std::sin(m_impl->lfoPhase * 2.0f * 3.14159265f) + 1.0f) * 0.5f;
        m_impl->lfoPhase += m_impl->lfoIncrement;
        if (m_impl->lfoPhase >= 1.0f) m_impl->lfoPhase -= 1.0f;

        float delaySamples = (m_impl->delayMs / 1000.0f) * static_cast<float>(m_impl->sampleRate);
        float modulatedDelay = delaySamples * (1.0f + lfo * m_impl->depth);

        float readIndexF = static_cast<float>(m_impl->writeIndex) - modulatedDelay;
        if (readIndexF < 0) readIndexF += static_cast<float>(m_impl->delayBufferL.size());

        std::size_t readIndex0 = static_cast<std::size_t>(readIndexF) % m_impl->delayBufferL.size();
        std::size_t readIndex1 = (readIndex0 + 1) % m_impl->delayBufferL.size();
        float frac = readIndexF - std::floor(readIndexF);

        float delayedL = m_impl->delayBufferL[readIndex0] * (1.0f - frac) + m_impl->delayBufferL[readIndex1] * frac;
        m_impl->delayBufferL[m_impl->writeIndex] = inputBuffer[i * numChannels] + delayedL * m_impl->feedback;
        outputBuffer[i * numChannels] = inputBuffer[i * numChannels] * (1.0f - m_impl->mix) + delayedL * m_impl->mix;

        if (numChannels > 1) {
            float delayedR = m_impl->delayBufferR[readIndex0] * (1.0f - frac) + m_impl->delayBufferR[readIndex1] * frac;
            m_impl->delayBufferR[m_impl->writeIndex] = inputBuffer[i * numChannels + 1] + delayedR * m_impl->feedback;
            outputBuffer[i * numChannels + 1] = inputBuffer[i * numChannels + 1] * (1.0f - m_impl->mix) + delayedR * m_impl->mix;
        }

        m_impl->writeIndex = (m_impl->writeIndex + 1) % m_impl->delayBufferL.size();
    }
}

void Flanger::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->updateLfoIncrement();
    std::size_t maxDelaySamples = static_cast<std::size_t>(0.02 * sampleRate);
    m_impl->delayBufferL.resize(maxDelaySamples, 0.0f);
    m_impl->delayBufferR.resize(maxDelaySamples, 0.0f);
}

void Flanger::reset()
{
    m_impl->lfoPhase = 0.0f;
    m_impl->writeIndex = 0;
    std::fill(m_impl->delayBufferL.begin(), m_impl->delayBufferL.end(), 0.0f);
    std::fill(m_impl->delayBufferR.begin(), m_impl->delayBufferR.end(), 0.0f);
}

std::string Flanger::getNodeId() const { return m_impl->nodeId; }
std::string Flanger::getTypeName() const { return "Flanger"; }
std::uint32_t Flanger::getNumInputChannels() const { return 2; }
std::uint32_t Flanger::getNumOutputChannels() const { return 2; }
bool Flanger::isBypassed() const { return m_impl->bypassed; }
void Flanger::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void Flanger::setRate(float rateHz) { m_impl->rate = rateHz; m_impl->updateLfoIncrement(); }
float Flanger::getRate() const { return m_impl->rate; }
void Flanger::setDepth(float depth) { m_impl->depth = std::max(0.0f, std::min(1.0f, depth)); }
float Flanger::getDepth() const { return m_impl->depth; }
void Flanger::setFeedback(float feedback) { m_impl->feedback = std::max(-0.99f, std::min(0.99f, feedback)); }
float Flanger::getFeedback() const { return m_impl->feedback; }
void Flanger::setDelay(float delayMs) { m_impl->delayMs = std::max(0.1f, std::min(20.0f, delayMs)); }
float Flanger::getDelay() const { return m_impl->delayMs; }
void Flanger::setMix(float mix) { m_impl->mix = std::max(0.0f, std::min(1.0f, mix)); }
float Flanger::getMix() const { return m_impl->mix; }

} // namespace nap
