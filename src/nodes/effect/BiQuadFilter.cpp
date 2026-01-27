#include "BiQuadFilter.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class BiQuadFilter::Impl {
public:
    std::string nodeId;
    FilterType filterType = FilterType::LowPass;
    float frequency = 1000.0f;
    float q = 0.707f;
    float gainDb = 0.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    // Coefficients
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // State variables (stereo)
    float x1L = 0.0f, x2L = 0.0f, y1L = 0.0f, y2L = 0.0f;
    float x1R = 0.0f, x2R = 0.0f, y1R = 0.0f, y2R = 0.0f;

    void calculateCoefficients();
};

void BiQuadFilter::Impl::calculateCoefficients()
{
    float A = std::pow(10.0f, gainDb / 40.0f);
    float w0 = 2.0f * 3.14159265358979f * frequency / static_cast<float>(sampleRate);
    float cosW0 = std::cos(w0);
    float sinW0 = std::sin(w0);
    float alpha = sinW0 / (2.0f * q);

    float a0;

    switch (filterType) {
        case FilterType::LowPass:
            b0 = (1.0f - cosW0) / 2.0f;
            b1 = 1.0f - cosW0;
            b2 = (1.0f - cosW0) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosW0;
            a2 = 1.0f - alpha;
            break;
        case FilterType::HighPass:
            b0 = (1.0f + cosW0) / 2.0f;
            b1 = -(1.0f + cosW0);
            b2 = (1.0f + cosW0) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosW0;
            a2 = 1.0f - alpha;
            break;
        case FilterType::BandPass:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosW0;
            a2 = 1.0f - alpha;
            break;
        case FilterType::Notch:
            b0 = 1.0f;
            b1 = -2.0f * cosW0;
            b2 = 1.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cosW0;
            a2 = 1.0f - alpha;
            break;
        default:
            b0 = 1.0f; b1 = 0.0f; b2 = 0.0f;
            a0 = 1.0f; a1 = 0.0f; a2 = 0.0f;
            break;
    }

    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;
}

BiQuadFilter::BiQuadFilter()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "BiQuadFilter_" + std::to_string(++instanceCounter);
    m_impl->calculateCoefficients();
}

BiQuadFilter::~BiQuadFilter() = default;

BiQuadFilter::BiQuadFilter(BiQuadFilter&&) noexcept = default;
BiQuadFilter& BiQuadFilter::operator=(BiQuadFilter&&) noexcept = default;

void BiQuadFilter::process(const float* inputBuffer, float* outputBuffer,
                            std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        // Left channel
        float inL = inputBuffer[i * numChannels];
        float outL = m_impl->b0 * inL + m_impl->b1 * m_impl->x1L + m_impl->b2 * m_impl->x2L
                   - m_impl->a1 * m_impl->y1L - m_impl->a2 * m_impl->y2L;
        m_impl->x2L = m_impl->x1L; m_impl->x1L = inL;
        m_impl->y2L = m_impl->y1L; m_impl->y1L = outL;
        outputBuffer[i * numChannels] = outL;

        if (numChannels > 1) {
            float inR = inputBuffer[i * numChannels + 1];
            float outR = m_impl->b0 * inR + m_impl->b1 * m_impl->x1R + m_impl->b2 * m_impl->x2R
                       - m_impl->a1 * m_impl->y1R - m_impl->a2 * m_impl->y2R;
            m_impl->x2R = m_impl->x1R; m_impl->x1R = inR;
            m_impl->y2R = m_impl->y1R; m_impl->y1R = outR;
            outputBuffer[i * numChannels + 1] = outR;
        }
    }
}

void BiQuadFilter::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->calculateCoefficients();
}

void BiQuadFilter::reset()
{
    m_impl->x1L = m_impl->x2L = m_impl->y1L = m_impl->y2L = 0.0f;
    m_impl->x1R = m_impl->x2R = m_impl->y1R = m_impl->y2R = 0.0f;
}

std::string BiQuadFilter::getNodeId() const { return m_impl->nodeId; }
std::string BiQuadFilter::getTypeName() const { return "BiQuadFilter"; }
std::uint32_t BiQuadFilter::getNumInputChannels() const { return 2; }
std::uint32_t BiQuadFilter::getNumOutputChannels() const { return 2; }
bool BiQuadFilter::isBypassed() const { return m_impl->bypassed; }
void BiQuadFilter::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void BiQuadFilter::setFilterType(FilterType type) { m_impl->filterType = type; m_impl->calculateCoefficients(); }
BiQuadFilter::FilterType BiQuadFilter::getFilterType() const { return m_impl->filterType; }
void BiQuadFilter::setFrequency(float frequencyHz) { m_impl->frequency = frequencyHz; m_impl->calculateCoefficients(); }
float BiQuadFilter::getFrequency() const { return m_impl->frequency; }
void BiQuadFilter::setQ(float q) { m_impl->q = q; m_impl->calculateCoefficients(); }
float BiQuadFilter::getQ() const { return m_impl->q; }
void BiQuadFilter::setGain(float gainDb) { m_impl->gainDb = gainDb; m_impl->calculateCoefficients(); }
float BiQuadFilter::getGain() const { return m_impl->gainDb; }

} // namespace nap
