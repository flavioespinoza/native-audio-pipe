#include "Phaser.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace nap {

class Phaser::Impl {
public:
    std::string nodeId;
    float rate = 0.5f;
    float depth = 0.7f;
    float feedback = 0.5f;
    std::uint32_t stages = 4;
    float mix = 0.5f;
    float lfoPhase = 0.0f;
    float lfoIncrement = 0.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    static constexpr int maxStages = 12;
    std::array<float, maxStages> allpassDelaysL{};
    std::array<float, maxStages> allpassDelaysR{};
    float feedbackL = 0.0f;
    float feedbackR = 0.0f;

    void updateLfoIncrement() {
        lfoIncrement = rate / static_cast<float>(sampleRate);
    }
};

Phaser::Phaser()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "Phaser_" + std::to_string(++instanceCounter);
    m_impl->updateLfoIncrement();
}

Phaser::~Phaser() = default;

Phaser::Phaser(Phaser&&) noexcept = default;
Phaser& Phaser::operator=(Phaser&&) noexcept = default;

void Phaser::process(const float* inputBuffer, float* outputBuffer,
                      std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float lfo = std::sin(m_impl->lfoPhase * 2.0f * 3.14159265f);
        m_impl->lfoPhase += m_impl->lfoIncrement;
        if (m_impl->lfoPhase >= 1.0f) m_impl->lfoPhase -= 1.0f;

        float d = (1.0f + lfo * m_impl->depth) * 0.5f;
        float minFreq = 200.0f;
        float maxFreq = 1600.0f;
        float freq = minFreq + d * (maxFreq - minFreq);
        float coeff = (1.0f - freq / static_cast<float>(m_impl->sampleRate)) /
                      (1.0f + freq / static_cast<float>(m_impl->sampleRate));

        float inL = inputBuffer[i * numChannels] + m_impl->feedbackL * m_impl->feedback;
        float outL = inL;
        for (std::uint32_t s = 0; s < m_impl->stages; ++s) {
            float ap = coeff * (outL - m_impl->allpassDelaysL[s]) + m_impl->allpassDelaysL[s];
            m_impl->allpassDelaysL[s] = outL;
            outL = ap;
        }
        m_impl->feedbackL = outL;
        outputBuffer[i * numChannels] = inputBuffer[i * numChannels] * (1.0f - m_impl->mix) + outL * m_impl->mix;

        if (numChannels > 1) {
            float inR = inputBuffer[i * numChannels + 1] + m_impl->feedbackR * m_impl->feedback;
            float outR = inR;
            for (std::uint32_t s = 0; s < m_impl->stages; ++s) {
                float ap = coeff * (outR - m_impl->allpassDelaysR[s]) + m_impl->allpassDelaysR[s];
                m_impl->allpassDelaysR[s] = outR;
                outR = ap;
            }
            m_impl->feedbackR = outR;
            outputBuffer[i * numChannels + 1] = inputBuffer[i * numChannels + 1] * (1.0f - m_impl->mix) + outR * m_impl->mix;
        }
    }
}

void Phaser::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->updateLfoIncrement();
}

void Phaser::reset()
{
    m_impl->lfoPhase = 0.0f;
    m_impl->allpassDelaysL.fill(0.0f);
    m_impl->allpassDelaysR.fill(0.0f);
    m_impl->feedbackL = 0.0f;
    m_impl->feedbackR = 0.0f;
}

std::string Phaser::getNodeId() const { return m_impl->nodeId; }
std::string Phaser::getTypeName() const { return "Phaser"; }
std::uint32_t Phaser::getNumInputChannels() const { return 2; }
std::uint32_t Phaser::getNumOutputChannels() const { return 2; }
bool Phaser::isBypassed() const { return m_impl->bypassed; }
void Phaser::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void Phaser::setRate(float rateHz) { m_impl->rate = rateHz; m_impl->updateLfoIncrement(); }
float Phaser::getRate() const { return m_impl->rate; }
void Phaser::setDepth(float depth) { m_impl->depth = std::max(0.0f, std::min(1.0f, depth)); }
float Phaser::getDepth() const { return m_impl->depth; }
void Phaser::setFeedback(float feedback) { m_impl->feedback = std::max(-0.99f, std::min(0.99f, feedback)); }
float Phaser::getFeedback() const { return m_impl->feedback; }
void Phaser::setStages(std::uint32_t stages) { m_impl->stages = std::min(stages, static_cast<std::uint32_t>(Impl::maxStages)); }
std::uint32_t Phaser::getStages() const { return m_impl->stages; }
void Phaser::setMix(float mix) { m_impl->mix = std::max(0.0f, std::min(1.0f, mix)); }
float Phaser::getMix() const { return m_impl->mix; }

} // namespace nap
