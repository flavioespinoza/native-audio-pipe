#include "BiQuadFilter.h"
#include "../../core/parameters/FloatParameter.h"
#include "../../core/parameters/EnumParameter.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace nap {

class BiQuadFilter::Impl {
public:
    std::string nodeId;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    // Coefficients (normalized: a0 == 1.0 after calculateCoefficients)
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // Transposed Direct Form II state — 2 delay vars per channel
    float z1L = 0.0f, z2L = 0.0f;
    float z1R = 0.0f, z2R = 0.0f;

    // IParameter-backed parameter objects
    std::unique_ptr<EnumParameter>  paramFilterType;
    std::unique_ptr<FloatParameter> paramFrequency;
    std::unique_ptr<FloatParameter> paramQ;
    std::unique_ptr<FloatParameter> paramGain;

    void calculateCoefficients();
};

void BiQuadFilter::Impl::calculateCoefficients()
{
    const float frequency = paramFrequency->getValue();
    const float q         = paramQ->getValue();
    const float gainDb    = paramGain->getValue();
    const FilterType filterType = static_cast<FilterType>(paramFilterType->getSelectedIndex());

    (void)gainDb;  // used by PeakingEQ / Shelf types (not yet implemented in switch)
    float w0 = 2.0f * 3.14159265358979323846f * frequency / static_cast<float>(sampleRate);
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

    // Normalize so that a0 == 1.0
    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;
}

BiQuadFilter::BiQuadFilter()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "BiQuadFilter_" + std::to_string(++instanceCounter);

    // Wire up IParameter-backed parameters
    m_impl->paramFilterType = std::make_unique<EnumParameter>(
        "filterType",
        {"LowPass", "HighPass", "BandPass", "Notch", "AllPass", "PeakingEQ", "LowShelf", "HighShelf"},
        0  // default: LowPass
    );
    m_impl->paramFrequency = std::make_unique<FloatParameter>(
        "frequency", 1000.0f, 20.0f, 20000.0f
    );
    m_impl->paramQ = std::make_unique<FloatParameter>(
        "q", 0.707f, 0.1f, 10.0f
    );
    m_impl->paramGain = std::make_unique<FloatParameter>(
        "gain", 0.0f, -24.0f, 24.0f
    );

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

    // Cache coefficients in locals for the hot loop — avoids repeated pointer
    // chasing through m_impl on every iteration.
    const float b0 = m_impl->b0, b1 = m_impl->b1, b2 = m_impl->b2;
    const float a1 = m_impl->a1, a2 = m_impl->a2;

    // Transposed Direct Form II — 2 state vars per channel, zero allocation.
    //
    //   y[n] = b0 * x[n] + z1
    //   z1   = b1 * x[n] - a1 * y[n] + z2
    //   z2   = b2 * x[n] - a2 * y[n]
    //
    // This structure is numerically superior to Direct Form I at low
    // cutoff frequencies with single-precision float because feedback and
    // feedforward paths share the same delay chain, reducing coefficient
    // quantization error accumulation.

    float z1L = m_impl->z1L, z2L = m_impl->z2L;

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float x = inputBuffer[i * numChannels];
        float y = b0 * x + z1L;
        z1L = b1 * x - a1 * y + z2L;
        z2L = b2 * x - a2 * y;
        outputBuffer[i * numChannels] = y;
    }

    m_impl->z1L = z1L;
    m_impl->z2L = z2L;

    if (numChannels > 1) {
        float z1R = m_impl->z1R, z2R = m_impl->z2R;

        for (std::uint32_t i = 0; i < numFrames; ++i) {
            float x = inputBuffer[i * numChannels + 1];
            float y = b0 * x + z1R;
            z1R = b1 * x - a1 * y + z2R;
            z2R = b2 * x - a2 * y;
            outputBuffer[i * numChannels + 1] = y;
        }

        m_impl->z1R = z1R;
        m_impl->z2R = z2R;
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
    m_impl->z1L = m_impl->z2L = 0.0f;
    m_impl->z1R = m_impl->z2R = 0.0f;
}

std::string BiQuadFilter::getNodeId() const { return m_impl->nodeId; }
std::string BiQuadFilter::getTypeName() const { return "BiQuadFilter"; }
std::uint32_t BiQuadFilter::getNumInputChannels() const { return 2; }
std::uint32_t BiQuadFilter::getNumOutputChannels() const { return 2; }
bool BiQuadFilter::isBypassed() const { return m_impl->bypassed; }
void BiQuadFilter::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void BiQuadFilter::setFilterType(FilterType type)
{
    m_impl->paramFilterType->setSelectedIndex(static_cast<size_t>(type));
    m_impl->calculateCoefficients();
}
BiQuadFilter::FilterType BiQuadFilter::getFilterType() const
{
    return static_cast<FilterType>(m_impl->paramFilterType->getSelectedIndex());
}

void BiQuadFilter::setFrequency(float frequencyHz)
{
    m_impl->paramFrequency->setValue(frequencyHz);
    m_impl->calculateCoefficients();
}
float BiQuadFilter::getFrequency() const { return m_impl->paramFrequency->getValue(); }

void BiQuadFilter::setQ(float q)
{
    m_impl->paramQ->setValue(q);
    m_impl->calculateCoefficients();
}
float BiQuadFilter::getQ() const { return m_impl->paramQ->getValue(); }

void BiQuadFilter::setGain(float gainDb)
{
    m_impl->paramGain->setValue(gainDb);
    m_impl->calculateCoefficients();
}
float BiQuadFilter::getGain() const { return m_impl->paramGain->getValue(); }

} // namespace nap
