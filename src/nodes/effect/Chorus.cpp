#include "Chorus.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace nap {

class Chorus::Impl {
public:
    std::string nodeId;
    float rate = 1.5f;
    float depth = 0.5f;
    float delayMs = 25.0f;
    std::uint32_t voices = 3;
    float mix = 0.5f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    static constexpr int maxVoices = 8;
    std::array<float, maxVoices> lfoPhases{};
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    std::size_t writeIndex = 0;
};

Chorus::Chorus()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "Chorus_" + std::to_string(++instanceCounter);
    for (int i = 0; i < Impl::maxVoices; ++i) {
        m_impl->lfoPhases[i] = static_cast<float>(i) / Impl::maxVoices;
    }
}

Chorus::~Chorus() = default;

Chorus::Chorus(Chorus&&) noexcept = default;
Chorus& Chorus::operator=(Chorus&&) noexcept = default;

void Chorus::process(const float* inputBuffer, float* outputBuffer,
                      std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    float lfoIncrement = m_impl->rate / static_cast<float>(m_impl->sampleRate);

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        m_impl->delayBufferL[m_impl->writeIndex] = inputBuffer[i * numChannels];
        if (numChannels > 1) {
            m_impl->delayBufferR[m_impl->writeIndex] = inputBuffer[i * numChannels + 1];
        }

        float sumL = 0.0f, sumR = 0.0f;

        for (std::uint32_t v = 0; v < m_impl->voices; ++v) {
            float lfo = (std::sin(m_impl->lfoPhases[v] * 2.0f * 3.14159265f) + 1.0f) * 0.5f;
            m_impl->lfoPhases[v] += lfoIncrement;
            if (m_impl->lfoPhases[v] >= 1.0f) m_impl->lfoPhases[v] -= 1.0f;

            float delaySamples = (m_impl->delayMs / 1000.0f) * static_cast<float>(m_impl->sampleRate);
            float modulatedDelay = delaySamples * (1.0f + lfo * m_impl->depth * 0.5f);

            float readIndexF = static_cast<float>(m_impl->writeIndex) - modulatedDelay;
            if (readIndexF < 0) readIndexF += static_cast<float>(m_impl->delayBufferL.size());

            std::size_t readIndex0 = static_cast<std::size_t>(readIndexF) % m_impl->delayBufferL.size();
            std::size_t readIndex1 = (readIndex0 + 1) % m_impl->delayBufferL.size();
            float frac = readIndexF - std::floor(readIndexF);

            sumL += m_impl->delayBufferL[readIndex0] * (1.0f - frac) + m_impl->delayBufferL[readIndex1] * frac;
            if (numChannels > 1) {
                sumR += m_impl->delayBufferR[readIndex0] * (1.0f - frac) + m_impl->delayBufferR[readIndex1] * frac;
            }
        }

        sumL /= static_cast<float>(m_impl->voices);
        sumR /= static_cast<float>(m_impl->voices);

        outputBuffer[i * numChannels] = inputBuffer[i * numChannels] * (1.0f - m_impl->mix) + sumL * m_impl->mix;
        if (numChannels > 1) {
            outputBuffer[i * numChannels + 1] = inputBuffer[i * numChannels + 1] * (1.0f - m_impl->mix) + sumR * m_impl->mix;
        }

        m_impl->writeIndex = (m_impl->writeIndex + 1) % m_impl->delayBufferL.size();
    }
}

void Chorus::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    std::size_t maxDelaySamples = static_cast<std::size_t>(0.1 * sampleRate);
    m_impl->delayBufferL.resize(maxDelaySamples, 0.0f);
    m_impl->delayBufferR.resize(maxDelaySamples, 0.0f);
}

void Chorus::reset()
{
    m_impl->writeIndex = 0;
    std::fill(m_impl->delayBufferL.begin(), m_impl->delayBufferL.end(), 0.0f);
    std::fill(m_impl->delayBufferR.begin(), m_impl->delayBufferR.end(), 0.0f);
}

std::string Chorus::getNodeId() const { return m_impl->nodeId; }
std::string Chorus::getTypeName() const { return "Chorus"; }
std::uint32_t Chorus::getNumInputChannels() const { return 2; }
std::uint32_t Chorus::getNumOutputChannels() const { return 2; }
bool Chorus::isBypassed() const { return m_impl->bypassed; }
void Chorus::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void Chorus::setRate(float rateHz) { m_impl->rate = rateHz; }
float Chorus::getRate() const { return m_impl->rate; }
void Chorus::setDepth(float depth) { m_impl->depth = std::max(0.0f, std::min(1.0f, depth)); }
float Chorus::getDepth() const { return m_impl->depth; }
void Chorus::setDelay(float delayMs) { m_impl->delayMs = std::max(1.0f, std::min(50.0f, delayMs)); }
float Chorus::getDelay() const { return m_impl->delayMs; }
void Chorus::setVoices(std::uint32_t voices) { m_impl->voices = std::max(1u, std::min(static_cast<std::uint32_t>(Impl::maxVoices), voices)); }
std::uint32_t Chorus::getVoices() const { return m_impl->voices; }
void Chorus::setMix(float mix) { m_impl->mix = std::max(0.0f, std::min(1.0f, mix)); }
float Chorus::getMix() const { return m_impl->mix; }

} // namespace nap
