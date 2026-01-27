#include "HardClipper.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class HardClipper::Impl {
public:
    std::string nodeId;
    float threshold = 0.8f;
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
    float inputGainLinear = 1.0f;
    float outputGainLinear = 1.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

HardClipper::HardClipper()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "HardClipper_" + std::to_string(++instanceCounter);
}

HardClipper::~HardClipper() = default;

HardClipper::HardClipper(HardClipper&&) noexcept = default;
HardClipper& HardClipper::operator=(HardClipper&&) noexcept = default;

void HardClipper::process(const float* inputBuffer, float* outputBuffer,
                           std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
        float sample = inputBuffer[i] * m_impl->inputGainLinear;
        sample = std::max(-m_impl->threshold, std::min(m_impl->threshold, sample));
        outputBuffer[i] = sample * m_impl->outputGainLinear;
    }
}

void HardClipper::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void HardClipper::reset() {}

std::string HardClipper::getNodeId() const { return m_impl->nodeId; }
std::string HardClipper::getTypeName() const { return "HardClipper"; }
std::uint32_t HardClipper::getNumInputChannels() const { return 2; }
std::uint32_t HardClipper::getNumOutputChannels() const { return 2; }
bool HardClipper::isBypassed() const { return m_impl->bypassed; }
void HardClipper::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void HardClipper::setThreshold(float threshold) { m_impl->threshold = std::max(0.0f, std::min(1.0f, threshold)); }
float HardClipper::getThreshold() const { return m_impl->threshold; }

void HardClipper::setInputGain(float gainDb)
{
    m_impl->inputGainDb = gainDb;
    m_impl->inputGainLinear = std::pow(10.0f, gainDb / 20.0f);
}

float HardClipper::getInputGain() const { return m_impl->inputGainDb; }

void HardClipper::setOutputGain(float gainDb)
{
    m_impl->outputGainDb = gainDb;
    m_impl->outputGainLinear = std::pow(10.0f, gainDb / 20.0f);
}

float HardClipper::getOutputGain() const { return m_impl->outputGainDb; }

} // namespace nap
