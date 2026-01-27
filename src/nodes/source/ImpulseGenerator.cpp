#include "ImpulseGenerator.h"
#include <algorithm>
#include <string>

namespace nap {

class ImpulseGenerator::Impl {
public:
    std::string nodeId;
    float amplitude = 1.0f;
    std::uint32_t intervalSamples = 44100;
    std::uint32_t sampleCounter = 0;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
    bool active = false;
    bool oneShot = false;
    bool triggered = false;
    bool hasFired = false;
};

ImpulseGenerator::ImpulseGenerator()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "ImpulseGenerator_" + std::to_string(++instanceCounter);
}

ImpulseGenerator::~ImpulseGenerator() = default;

ImpulseGenerator::ImpulseGenerator(ImpulseGenerator&&) noexcept = default;
ImpulseGenerator& ImpulseGenerator::operator=(ImpulseGenerator&&) noexcept = default;

void ImpulseGenerator::process(const float* /*inputBuffer*/, float* outputBuffer,
                                std::uint32_t numFrames, std::uint32_t numChannels)
{
    generate(outputBuffer, numFrames, numChannels);
}

void ImpulseGenerator::generate(float* outputBuffer, std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (!m_impl->active || m_impl->bypassed) {
        std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float sample = 0.0f;

        if (m_impl->triggered) {
            sample = m_impl->amplitude;
            m_impl->triggered = false;
            m_impl->hasFired = true;
            m_impl->sampleCounter = 0;
        } else if (!m_impl->oneShot || !m_impl->hasFired) {
            m_impl->sampleCounter++;
            if (m_impl->sampleCounter >= m_impl->intervalSamples) {
                sample = m_impl->amplitude;
                m_impl->sampleCounter = 0;
                m_impl->hasFired = true;
            }
        }

        for (std::uint32_t ch = 0; ch < numChannels; ++ch) {
            outputBuffer[i * numChannels + ch] = sample;
        }
    }
}

void ImpulseGenerator::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void ImpulseGenerator::reset()
{
    m_impl->sampleCounter = 0;
    m_impl->triggered = false;
    m_impl->hasFired = false;
}

std::string ImpulseGenerator::getNodeId() const { return m_impl->nodeId; }
std::string ImpulseGenerator::getTypeName() const { return "ImpulseGenerator"; }
std::uint32_t ImpulseGenerator::getNumInputChannels() const { return 0; }
std::uint32_t ImpulseGenerator::getNumOutputChannels() const { return 2; }
bool ImpulseGenerator::isBypassed() const { return m_impl->bypassed; }
void ImpulseGenerator::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

bool ImpulseGenerator::hasMoreData() const { return m_impl->active && (!m_impl->oneShot || !m_impl->hasFired); }
std::uint64_t ImpulseGenerator::getTotalSamples() const { return 0; }
std::uint64_t ImpulseGenerator::getCurrentPosition() const { return 0; }
bool ImpulseGenerator::seek(std::uint64_t /*positionSamples*/) { return false; }
bool ImpulseGenerator::isSeekable() const { return false; }
void ImpulseGenerator::start() { m_impl->active = true; }
void ImpulseGenerator::stop() { m_impl->active = false; }
bool ImpulseGenerator::isActive() const { return m_impl->active; }

void ImpulseGenerator::setAmplitude(float amplitude) { m_impl->amplitude = amplitude; }
float ImpulseGenerator::getAmplitude() const { return m_impl->amplitude; }
void ImpulseGenerator::setIntervalSamples(std::uint32_t samples) { m_impl->intervalSamples = samples; }
std::uint32_t ImpulseGenerator::getIntervalSamples() const { return m_impl->intervalSamples; }

void ImpulseGenerator::setIntervalMs(float milliseconds)
{
    m_impl->intervalSamples = static_cast<std::uint32_t>(milliseconds * m_impl->sampleRate / 1000.0);
}

float ImpulseGenerator::getIntervalMs() const
{
    return static_cast<float>(m_impl->intervalSamples) * 1000.0f / static_cast<float>(m_impl->sampleRate);
}

void ImpulseGenerator::trigger() { m_impl->triggered = true; }
void ImpulseGenerator::setOneShot(bool oneShot) { m_impl->oneShot = oneShot; }
bool ImpulseGenerator::isOneShot() const { return m_impl->oneShot; }

} // namespace nap
