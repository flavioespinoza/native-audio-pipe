#include "ReverbConvolution.h"
#include <algorithm>
#include <vector>

namespace nap {

class ReverbConvolution::Impl {
public:
    std::string nodeId;
    std::vector<float> impulseResponse;
    std::vector<float> inputBuffer;
    std::size_t inputIndex = 0;
    float dryWetMix = 0.5f;
    float preDelayMs = 0.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
    bool irLoaded = false;
};

ReverbConvolution::ReverbConvolution()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "ReverbConvolution_" + std::to_string(++instanceCounter);
}

ReverbConvolution::~ReverbConvolution() = default;

ReverbConvolution::ReverbConvolution(ReverbConvolution&&) noexcept = default;
ReverbConvolution& ReverbConvolution::operator=(ReverbConvolution&&) noexcept = default;

void ReverbConvolution::process(const float* inputBuffer, float* outputBuffer,
                                 std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed || !m_impl->irLoaded) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    // Simplified convolution (direct form, not optimized)
    for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
        m_impl->inputBuffer[m_impl->inputIndex] = inputBuffer[i];

        float wetSample = 0.0f;
        std::size_t irLen = std::min(m_impl->impulseResponse.size(), m_impl->inputBuffer.size());

        for (std::size_t j = 0; j < irLen; ++j) {
            std::size_t idx = (m_impl->inputIndex + m_impl->inputBuffer.size() - j) % m_impl->inputBuffer.size();
            wetSample += m_impl->inputBuffer[idx] * m_impl->impulseResponse[j];
        }

        outputBuffer[i] = inputBuffer[i] * (1.0f - m_impl->dryWetMix) + wetSample * m_impl->dryWetMix;

        m_impl->inputIndex = (m_impl->inputIndex + 1) % m_impl->inputBuffer.size();
    }
}

void ReverbConvolution::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;

    // Resize input buffer to match IR length
    if (m_impl->irLoaded) {
        m_impl->inputBuffer.resize(m_impl->impulseResponse.size(), 0.0f);
    }
}

void ReverbConvolution::reset()
{
    std::fill(m_impl->inputBuffer.begin(), m_impl->inputBuffer.end(), 0.0f);
    m_impl->inputIndex = 0;
}

std::string ReverbConvolution::getNodeId() const { return m_impl->nodeId; }
std::string ReverbConvolution::getTypeName() const { return "ReverbConvolution"; }
std::uint32_t ReverbConvolution::getNumInputChannels() const { return 2; }
std::uint32_t ReverbConvolution::getNumOutputChannels() const { return 2; }
bool ReverbConvolution::isBypassed() const { return m_impl->bypassed; }
void ReverbConvolution::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

bool ReverbConvolution::loadImpulseResponse(const std::string& /*filePath*/)
{
    // TODO: Implement file loading
    return false;
}

bool ReverbConvolution::loadImpulseResponse(const float* irData, std::size_t irLength)
{
    if (!irData || irLength == 0) {
        return false;
    }

    m_impl->impulseResponse.assign(irData, irData + irLength);
    m_impl->inputBuffer.resize(irLength, 0.0f);
    m_impl->irLoaded = true;
    return true;
}

void ReverbConvolution::unloadImpulseResponse()
{
    m_impl->impulseResponse.clear();
    m_impl->inputBuffer.clear();
    m_impl->irLoaded = false;
}

bool ReverbConvolution::isImpulseResponseLoaded() const { return m_impl->irLoaded; }
std::size_t ReverbConvolution::getImpulseResponseLength() const { return m_impl->impulseResponse.size(); }
void ReverbConvolution::setDryWetMix(float mix) { m_impl->dryWetMix = std::max(0.0f, std::min(1.0f, mix)); }
float ReverbConvolution::getDryWetMix() const { return m_impl->dryWetMix; }
void ReverbConvolution::setPreDelay(float milliseconds) { m_impl->preDelayMs = milliseconds; }
float ReverbConvolution::getPreDelay() const { return m_impl->preDelayMs; }

} // namespace nap
