#include "BiQuadFilter.h"
#include "../../core/parameters/FloatParameter.h"
#include "../../core/parameters/EnumParameter.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

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

    // State variables for Transposed Direct Form II (stereo)
    // Only 2 state variables per channel instead of 4
    float s1L = 0.0f, s2L = 0.0f;
    float s1R = 0.0f, s2R = 0.0f;

    // IParameter system integration
    std::unique_ptr<FloatParameter> frequencyParam;
    std::unique_ptr<FloatParameter> qParam;
    std::unique_ptr<EnumParameter> filterTypeParam;
    std::unique_ptr<FloatParameter> gainParam;

    // Track current smoothed values to detect changes
    float currentSmoothedFreq = 1000.0f;
    float currentSmoothedQ = 0.707f;
    float currentSmoothedGain = 0.0f;
    bool coefficientsNeedUpdate = false;

    void calculateCoefficients();
};

void BiQuadFilter::Impl::calculateCoefficients()
{
    // Safety clamp: never exceed 95% of Nyquist frequency
    // This prevents filter instability and aliasing artifacts
    const float nyquist = static_cast<float>(sampleRate) / 2.0f;
    const float maxSafeFreq = nyquist * 0.95f;  // 5% margin for numerical stability
    const float clampedFreq = std::min(frequency, maxSafeFreq);

    float A = std::pow(10.0f, gainDb / 40.0f);
    float w0 = 2.0f * 3.14159265358979f * clampedFreq / static_cast<float>(sampleRate);
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

    // Initialize IParameter system integration
    // Frequency: 20 Hz to 20 kHz (audible range)
    m_impl->frequencyParam = std::make_unique<FloatParameter>(
        "Frequency", 1000.0f, 20.0f, 20000.0f);
    m_impl->frequencyParam->enableSmoothing(true, 0.01f);  // 10ms smoothing time

    // Q: 0.1 to 20.0 (resonance/bandwidth control)
    m_impl->qParam = std::make_unique<FloatParameter>(
        "Q", 0.707f, 0.1f, 20.0f);
    m_impl->qParam->enableSmoothing(true, 0.01f);  // 10ms smoothing time

    // Filter Type: LowPass, HighPass, BandPass, Notch
    std::vector<std::string> filterTypeOptions = {
        "LowPass", "HighPass", "BandPass", "Notch"
    };
    m_impl->filterTypeParam = std::make_unique<EnumParameter>(
        "Filter Type", filterTypeOptions, 0);  // Default: LowPass

    // Gain: -24 dB to +24 dB (for Peaking EQ, Shelving filters)
    m_impl->gainParam = std::make_unique<FloatParameter>(
        "Gain", 0.0f, -24.0f, 24.0f);
    m_impl->gainParam->enableSmoothing(true, 0.01f);  // 10ms smoothing time

    // Wire up change callbacks - these fire when external systems change parameters
    // (automation, MIDI CC, preset load, UI interaction)
    m_impl->frequencyParam->setChangeCallback([this](float oldValue, float newValue) {
        m_impl->frequency = newValue;
        m_impl->coefficientsNeedUpdate = true;
    });

    m_impl->qParam->setChangeCallback([this](float oldValue, float newValue) {
        m_impl->q = newValue;
        m_impl->coefficientsNeedUpdate = true;
    });

    m_impl->filterTypeParam->setChangeCallback([this](size_t oldIndex, size_t newIndex) {
        m_impl->filterType = static_cast<FilterType>(newIndex);
        m_impl->coefficientsNeedUpdate = true;
    });

    m_impl->gainParam->setChangeCallback([this](float oldValue, float newValue) {
        m_impl->gainDb = newValue;
        m_impl->coefficientsNeedUpdate = true;
    });

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

    // Get smoothed parameter values ONCE per buffer (not per sample)
    // This prevents zipper noise while maintaining performance
    const float smoothedFreq = m_impl->frequencyParam->getSmoothedValue(
        static_cast<float>(m_impl->sampleRate));
    const float smoothedQ = m_impl->qParam->getSmoothedValue(
        static_cast<float>(m_impl->sampleRate));
    const float smoothedGain = m_impl->gainParam->getSmoothedValue(
        static_cast<float>(m_impl->sampleRate));

    // Check if smoothed values have changed enough to warrant coefficient recalculation
    // Use a small epsilon to avoid unnecessary recalculations due to floating point noise
    const float epsilon = 0.0001f;
    const bool freqChanged = std::abs(smoothedFreq - m_impl->currentSmoothedFreq) > epsilon;
    const bool qChanged = std::abs(smoothedQ - m_impl->currentSmoothedQ) > epsilon;
    const bool gainChanged = std::abs(smoothedGain - m_impl->currentSmoothedGain) > epsilon;

    if (freqChanged || qChanged || gainChanged || m_impl->coefficientsNeedUpdate) {
        // Update internal state with smoothed values
        m_impl->frequency = smoothedFreq;
        m_impl->q = smoothedQ;
        m_impl->gainDb = smoothedGain;

        // Store current smoothed values for next buffer
        m_impl->currentSmoothedFreq = smoothedFreq;
        m_impl->currentSmoothedQ = smoothedQ;
        m_impl->currentSmoothedGain = smoothedGain;

        // Recalculate filter coefficients once per buffer
        m_impl->calculateCoefficients();
        m_impl->coefficientsNeedUpdate = false;
    }

    // Transposed Direct Form II - more numerically stable than Direct Form I
    // Algorithm:
    //   y[n] = b0*x[n] + s1[n-1]
    //   s1[n] = b1*x[n] - a1*y[n] + s2[n-1]
    //   s2[n] = b2*x[n] - a2*y[n]

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        // Left channel
        const float inL = inputBuffer[i * numChannels];
        const float outL = m_impl->b0 * inL + m_impl->s1L;
        m_impl->s1L = m_impl->b1 * inL - m_impl->a1 * outL + m_impl->s2L;
        m_impl->s2L = m_impl->b2 * inL - m_impl->a2 * outL;
        outputBuffer[i * numChannels] = outL;

        if (numChannels > 1) {
            // Right channel
            const float inR = inputBuffer[i * numChannels + 1];
            const float outR = m_impl->b0 * inR + m_impl->s1R;
            m_impl->s1R = m_impl->b1 * inR - m_impl->a1 * outR + m_impl->s2R;
            m_impl->s2R = m_impl->b2 * inR - m_impl->a2 * outR;
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
    m_impl->s1L = m_impl->s2L = 0.0f;
    m_impl->s1R = m_impl->s2R = 0.0f;
}

std::string BiQuadFilter::getNodeId() const { return m_impl->nodeId; }
std::string BiQuadFilter::getTypeName() const { return "BiQuadFilter"; }
std::uint32_t BiQuadFilter::getNumInputChannels() const { return 2; }
std::uint32_t BiQuadFilter::getNumOutputChannels() const { return 2; }
bool BiQuadFilter::isBypassed() const { return m_impl->bypassed; }
void BiQuadFilter::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void BiQuadFilter::setFilterType(FilterType type) {
    // Update both internal state and parameter (syncs the two systems)
    m_impl->filterType = type;
    m_impl->filterTypeParam->setSelectedIndex(static_cast<size_t>(type));
}

BiQuadFilter::FilterType BiQuadFilter::getFilterType() const {
    return m_impl->filterType;
}

void BiQuadFilter::setFrequency(float frequencyHz) {
    // Update both internal state and parameter (syncs the two systems)
    m_impl->frequency = frequencyHz;
    m_impl->frequencyParam->setValue(frequencyHz);
}

float BiQuadFilter::getFrequency() const {
    return m_impl->frequency;
}

void BiQuadFilter::setQ(float q) {
    // Update both internal state and parameter (syncs the two systems)
    m_impl->q = q;
    m_impl->qParam->setValue(q);
}

float BiQuadFilter::getQ() const {
    return m_impl->q;
}

void BiQuadFilter::setGain(float gainDb) {
    // Update both internal state and parameter (syncs the two systems)
    m_impl->gainDb = gainDb;
    m_impl->gainParam->setValue(gainDb);
}

float BiQuadFilter::getGain() const {
    return m_impl->gainDb;
}

// IParameter system accessors - for automation, presets, UI binding
FloatParameter* BiQuadFilter::getFrequencyParameter() {
    return m_impl->frequencyParam.get();
}

FloatParameter* BiQuadFilter::getQParameter() {
    return m_impl->qParam.get();
}

EnumParameter* BiQuadFilter::getFilterTypeParameter() {
    return m_impl->filterTypeParam.get();
}

FloatParameter* BiQuadFilter::getGainParameter() {
    return m_impl->gainParam.get();
}

} // namespace nap
