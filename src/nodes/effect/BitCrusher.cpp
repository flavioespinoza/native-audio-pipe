#include "BitCrusher.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class BitCrusher::Impl {
public:
    std::string nodeId;
    std::uint32_t bitDepth = 8;
    std::uint32_t downsampleFactor = 1;
    float mix = 1.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
    std::uint32_t sampleCounter = 0;
    float holdSampleL = 0.0f;
    float holdSampleR = 0.0f;
};

BitCrusher::BitCrusher()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "BitCrusher_" + std::to_string(++instanceCounter);
}

BitCrusher::~BitCrusher() = default;

BitCrusher::BitCrusher(BitCrusher&&) noexcept = default;
BitCrusher& BitCrusher::operator=(BitCrusher&&) noexcept = default;

void BitCrusher::process(const float* inputBuffer, float* outputBuffer,
                          std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    float levels = std::pow(2.0f, static_cast<float>(m_impl->bitDepth));

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        if (m_impl->sampleCounter == 0) {
            float sampleL = inputBuffer[i * numChannels];
            m_impl->holdSampleL = std::round(sampleL * levels) / levels;

            if (numChannels > 1) {
                float sampleR = inputBuffer[i * numChannels + 1];
                m_impl->holdSampleR = std::round(sampleR * levels) / levels;
            }
        }

        m_impl->sampleCounter++;
        if (m_impl->sampleCounter >= m_impl->downsampleFactor) {
            m_impl->sampleCounter = 0;
        }

        outputBuffer[i * numChannels] = inputBuffer[i * numChannels] * (1.0f - m_impl->mix)
                                       + m_impl->holdSampleL * m_impl->mix;
        if (numChannels > 1) {
            outputBuffer[i * numChannels + 1] = inputBuffer[i * numChannels + 1] * (1.0f - m_impl->mix)
                                               + m_impl->holdSampleR * m_impl->mix;
        }
    }
}

void BitCrusher::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void BitCrusher::reset()
{
    m_impl->sampleCounter = 0;
    m_impl->holdSampleL = 0.0f;
    m_impl->holdSampleR = 0.0f;
}

std::string BitCrusher::getNodeId() const { return m_impl->nodeId; }
std::string BitCrusher::getTypeName() const { return "BitCrusher"; }
std::uint32_t BitCrusher::getNumInputChannels() const { return 2; }
std::uint32_t BitCrusher::getNumOutputChannels() const { return 2; }
bool BitCrusher::isBypassed() const { return m_impl->bypassed; }
void BitCrusher::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void BitCrusher::setBitDepth(std::uint32_t bits) { m_impl->bitDepth = std::max(1u, std::min(16u, bits)); }
std::uint32_t BitCrusher::getBitDepth() const { return m_impl->bitDepth; }
void BitCrusher::setDownsampleFactor(std::uint32_t factor) { m_impl->downsampleFactor = std::max(1u, factor); }
std::uint32_t BitCrusher::getDownsampleFactor() const { return m_impl->downsampleFactor; }
void BitCrusher::setMix(float mix) { m_impl->mix = std::max(0.0f, std::min(1.0f, mix)); }
float BitCrusher::getMix() const { return m_impl->mix; }

} // namespace nap
