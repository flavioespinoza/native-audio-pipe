#include "SineOscillator.h"
#include <cmath>
#include <string>

namespace nap {

class SineOscillator::Impl {
public:
    std::string nodeId;
    float frequency = 440.0f;
    float amplitude = 1.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
    bool active = false;

    void updatePhaseIncrement() {
        phaseIncrement = 2.0f * 3.14159265358979f * frequency / static_cast<float>(sampleRate);
    }
};

SineOscillator::SineOscillator()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "SineOscillator_" + std::to_string(++instanceCounter);
}

SineOscillator::~SineOscillator() = default;

SineOscillator::SineOscillator(SineOscillator&&) noexcept = default;
SineOscillator& SineOscillator::operator=(SineOscillator&&) noexcept = default;

void SineOscillator::process(const float* /*inputBuffer*/, float* outputBuffer,
                              std::uint32_t numFrames, std::uint32_t numChannels)
{
    generate(outputBuffer, numFrames, numChannels);
}

void SineOscillator::generate(float* outputBuffer, std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (!m_impl->active || m_impl->bypassed) {
        std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float sample = m_impl->amplitude * std::sin(m_impl->phase);
        m_impl->phase += m_impl->phaseIncrement;

        if (m_impl->phase >= 2.0f * 3.14159265358979f) {
            m_impl->phase -= 2.0f * 3.14159265358979f;
        }

        for (std::uint32_t ch = 0; ch < numChannels; ++ch) {
            outputBuffer[i * numChannels + ch] = sample;
        }
    }
}

void SineOscillator::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->updatePhaseIncrement();
}

void SineOscillator::reset()
{
    m_impl->phase = 0.0f;
}

std::string SineOscillator::getNodeId() const { return m_impl->nodeId; }
std::string SineOscillator::getTypeName() const { return "SineOscillator"; }
std::uint32_t SineOscillator::getNumInputChannels() const { return 0; }
std::uint32_t SineOscillator::getNumOutputChannels() const { return 2; }
bool SineOscillator::isBypassed() const { return m_impl->bypassed; }
void SineOscillator::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

bool SineOscillator::hasMoreData() const { return m_impl->active; }
std::uint64_t SineOscillator::getTotalSamples() const { return 0; }
std::uint64_t SineOscillator::getCurrentPosition() const { return 0; }
bool SineOscillator::seek(std::uint64_t /*positionSamples*/) { return false; }
bool SineOscillator::isSeekable() const { return false; }
void SineOscillator::start() { m_impl->active = true; }
void SineOscillator::stop() { m_impl->active = false; }
bool SineOscillator::isActive() const { return m_impl->active; }

void SineOscillator::setFrequency(float frequencyHz)
{
    m_impl->frequency = frequencyHz;
    m_impl->updatePhaseIncrement();
}

float SineOscillator::getFrequency() const { return m_impl->frequency; }
void SineOscillator::setAmplitude(float amplitude) { m_impl->amplitude = amplitude; }
float SineOscillator::getAmplitude() const { return m_impl->amplitude; }
void SineOscillator::setPhase(float phaseRadians) { m_impl->phase = phaseRadians; }
float SineOscillator::getPhase() const { return m_impl->phase; }

} // namespace nap
