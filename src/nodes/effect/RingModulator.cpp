#include "RingModulator.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class RingModulator::Impl {
public:
    std::string nodeId;
    float frequency = 440.0f;
    WaveformType waveform = WaveformType::Sine;
    float mix = 1.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    void updatePhaseIncrement() {
        phaseIncrement = frequency / static_cast<float>(sampleRate);
    }
};

RingModulator::RingModulator()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "RingModulator_" + std::to_string(++instanceCounter);
    m_impl->updatePhaseIncrement();
}

RingModulator::~RingModulator() = default;

RingModulator::RingModulator(RingModulator&&) noexcept = default;
RingModulator& RingModulator::operator=(RingModulator&&) noexcept = default;

void RingModulator::process(const float* inputBuffer, float* outputBuffer,
                             std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float modulator;
        switch (m_impl->waveform) {
            case WaveformType::Sine:
                modulator = std::sin(m_impl->phase * 2.0f * 3.14159265f);
                break;
            case WaveformType::Square:
                modulator = m_impl->phase < 0.5f ? 1.0f : -1.0f;
                break;
            case WaveformType::Triangle:
                modulator = 4.0f * std::abs(m_impl->phase - 0.5f) - 1.0f;
                break;
            case WaveformType::Sawtooth:
                modulator = 2.0f * m_impl->phase - 1.0f;
                break;
            default:
                modulator = 0.0f;
        }

        m_impl->phase += m_impl->phaseIncrement;
        if (m_impl->phase >= 1.0f) m_impl->phase -= 1.0f;

        for (std::uint32_t ch = 0; ch < numChannels; ++ch) {
            float input = inputBuffer[i * numChannels + ch];
            float modulated = input * modulator;
            outputBuffer[i * numChannels + ch] = input * (1.0f - m_impl->mix) + modulated * m_impl->mix;
        }
    }
}

void RingModulator::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->updatePhaseIncrement();
}

void RingModulator::reset() { m_impl->phase = 0.0f; }

std::string RingModulator::getNodeId() const { return m_impl->nodeId; }
std::string RingModulator::getTypeName() const { return "RingModulator"; }
std::uint32_t RingModulator::getNumInputChannels() const { return 2; }
std::uint32_t RingModulator::getNumOutputChannels() const { return 2; }
bool RingModulator::isBypassed() const { return m_impl->bypassed; }
void RingModulator::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void RingModulator::setFrequency(float frequencyHz) { m_impl->frequency = frequencyHz; m_impl->updatePhaseIncrement(); }
float RingModulator::getFrequency() const { return m_impl->frequency; }
void RingModulator::setWaveform(WaveformType type) { m_impl->waveform = type; }
RingModulator::WaveformType RingModulator::getWaveform() const { return m_impl->waveform; }
void RingModulator::setMix(float mix) { m_impl->mix = std::max(0.0f, std::min(1.0f, mix)); }
float RingModulator::getMix() const { return m_impl->mix; }

} // namespace nap
