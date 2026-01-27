#include "SoftClipper.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class SoftClipper::Impl {
public:
    std::string nodeId;
    float drive = 1.0f;
    float mix = 1.0f;
    CurveType curveType = CurveType::Tanh;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

SoftClipper::SoftClipper()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "SoftClipper_" + std::to_string(++instanceCounter);
}

SoftClipper::~SoftClipper() = default;

SoftClipper::SoftClipper(SoftClipper&&) noexcept = default;
SoftClipper& SoftClipper::operator=(SoftClipper&&) noexcept = default;

void SoftClipper::process(const float* inputBuffer, float* outputBuffer,
                           std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
        float sample = inputBuffer[i] * m_impl->drive;
        float processed;

        switch (m_impl->curveType) {
            case CurveType::Tanh:
                processed = std::tanh(sample);
                break;
            case CurveType::Atan:
                processed = (2.0f / 3.14159265f) * std::atan(sample);
                break;
            case CurveType::Cubic:
                processed = sample - (sample * sample * sample) / 3.0f;
                processed = std::max(-1.0f, std::min(1.0f, processed));
                break;
            case CurveType::Sine:
                processed = std::sin(sample * 1.5707963f);
                processed = std::max(-1.0f, std::min(1.0f, processed));
                break;
            default:
                processed = sample;
        }

        outputBuffer[i] = inputBuffer[i] * (1.0f - m_impl->mix) + processed * m_impl->mix;
    }
}

void SoftClipper::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void SoftClipper::reset() {}

std::string SoftClipper::getNodeId() const { return m_impl->nodeId; }
std::string SoftClipper::getTypeName() const { return "SoftClipper"; }
std::uint32_t SoftClipper::getNumInputChannels() const { return 2; }
std::uint32_t SoftClipper::getNumOutputChannels() const { return 2; }
bool SoftClipper::isBypassed() const { return m_impl->bypassed; }
void SoftClipper::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void SoftClipper::setDrive(float drive) { m_impl->drive = std::max(0.1f, drive); }
float SoftClipper::getDrive() const { return m_impl->drive; }
void SoftClipper::setCurveType(CurveType type) { m_impl->curveType = type; }
SoftClipper::CurveType SoftClipper::getCurveType() const { return m_impl->curveType; }
void SoftClipper::setMix(float mix) { m_impl->mix = std::max(0.0f, std::min(1.0f, mix)); }
float SoftClipper::getMix() const { return m_impl->mix; }

} // namespace nap
