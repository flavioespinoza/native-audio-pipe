#include "MeterNode.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace nap {

class MeterNode::Impl {
public:
    std::string nodeId;
    std::array<float, 2> peakLevels{0.0f, 0.0f};
    std::array<float, 2> rmsAccumulators{0.0f, 0.0f};
    std::array<float, 2> rmsLevels{0.0f, 0.0f};
    float decayRate = 0.9995f;
    std::uint32_t rmsWindowSize = 1024;
    std::uint32_t rmsSampleCount = 0;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

MeterNode::MeterNode()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "MeterNode_" + std::to_string(++instanceCounter);
}

MeterNode::~MeterNode() = default;

MeterNode::MeterNode(MeterNode&&) noexcept = default;
MeterNode& MeterNode::operator=(MeterNode&&) noexcept = default;

void MeterNode::process(const float* inputBuffer, float* outputBuffer,
                         std::uint32_t numFrames, std::uint32_t numChannels)
{
    std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);

    if (m_impl->bypassed) return;

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        for (std::uint32_t ch = 0; ch < std::min(numChannels, 2u); ++ch) {
            float sample = std::abs(inputBuffer[i * numChannels + ch]);

            if (sample > m_impl->peakLevels[ch]) {
                m_impl->peakLevels[ch] = sample;
            } else {
                m_impl->peakLevels[ch] *= m_impl->decayRate;
            }

            m_impl->rmsAccumulators[ch] += sample * sample;
        }

        m_impl->rmsSampleCount++;
        if (m_impl->rmsSampleCount >= m_impl->rmsWindowSize) {
            for (std::uint32_t ch = 0; ch < 2; ++ch) {
                m_impl->rmsLevels[ch] = std::sqrt(m_impl->rmsAccumulators[ch] / m_impl->rmsWindowSize);
                m_impl->rmsAccumulators[ch] = 0.0f;
            }
            m_impl->rmsSampleCount = 0;
        }
    }
}

void MeterNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->rmsWindowSize = static_cast<std::uint32_t>(sampleRate * 0.3); // 300ms window
}

void MeterNode::reset()
{
    m_impl->peakLevels.fill(0.0f);
    m_impl->rmsAccumulators.fill(0.0f);
    m_impl->rmsLevels.fill(0.0f);
    m_impl->rmsSampleCount = 0;
}

std::string MeterNode::getNodeId() const { return m_impl->nodeId; }
std::string MeterNode::getTypeName() const { return "MeterNode"; }
std::uint32_t MeterNode::getNumInputChannels() const { return 2; }
std::uint32_t MeterNode::getNumOutputChannels() const { return 2; }
bool MeterNode::isBypassed() const { return m_impl->bypassed; }
void MeterNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

float MeterNode::getPeakLevel(std::uint32_t channel) const
{
    return channel < 2 ? m_impl->peakLevels[channel] : 0.0f;
}

float MeterNode::getRmsLevel(std::uint32_t channel) const
{
    return channel < 2 ? m_impl->rmsLevels[channel] : 0.0f;
}

float MeterNode::getPeakLevelDb(std::uint32_t channel) const
{
    float level = getPeakLevel(channel);
    return level > 0.0f ? 20.0f * std::log10(level) : -100.0f;
}

float MeterNode::getRmsLevelDb(std::uint32_t channel) const
{
    float level = getRmsLevel(channel);
    return level > 0.0f ? 20.0f * std::log10(level) : -100.0f;
}

void MeterNode::resetPeaks()
{
    m_impl->peakLevels.fill(0.0f);
}

void MeterNode::setDecayRate(float decayPerSecond)
{
    m_impl->decayRate = std::pow(decayPerSecond, 1.0f / static_cast<float>(m_impl->sampleRate));
}

float MeterNode::getDecayRate() const { return m_impl->decayRate; }

} // namespace nap
