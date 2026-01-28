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
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    // Target coefficients — set by calculateCoefficients() whenever a
    // parameter changes (via change callback) or smoothed values shift.
    float targetB0 = 1.0f, targetB1 = 0.0f, targetB2 = 0.0f;
    float targetA1 = 0.0f, targetA2 = 0.0f;

    // Running coefficients — smoothed toward targets per-sample in process().
    // These are what the TDF-II loop actually uses.
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    // One-pole coefficient for per-sample coefficient smoothing.
    // = exp(-1 / (sampleRate * coeffSmoothingTimeSeconds))
    // Higher value = slower tracking (more smoothing).
    float coeffSmoothCoeff = 0.0f;

    // One-pole coefficient for advancing parameter smoothers in the hot loop.
    // Pre-cached so we never call exp() inside process().
    float paramSmoothCoeff = 0.0f;

    // Cached smoothed parameter values — evolved per-sample in process() using
    // paramSmoothCoeff.  Targets for the parameter smoothers are held inside
    // the FloatParameter objects (via getValue()).
    float smoothedFrequency = 1000.0f;
    float smoothedQ         = 0.707f;
    float smoothedGain      = 0.0f;

    // Transposed Direct Form II state — 2 delay vars per channel
    float z1L = 0.0f, z2L = 0.0f;
    float z1R = 0.0f, z2R = 0.0f;

    // IParameter-backed parameter objects
    std::unique_ptr<EnumParameter>  paramFilterType;
    std::unique_ptr<FloatParameter> paramFrequency;
    std::unique_ptr<FloatParameter> paramQ;
    std::unique_ptr<FloatParameter> paramGain;

    // Compute target coefficients from given frequency/Q/type.
    // Called both from change callbacks (with raw getValue()) and from
    // process() (with per-block smoothed values).
    void computeTargetCoefficients(float frequency, float q, FilterType type);

    // Convenience: read raw parameter values and update targets.
    // Wired as the change callback for all parameters — fires whenever
    // an external system (automation, MIDI CC, preset load) writes a value.
    void calculateCoefficients();

    // Recompute the cached smoothing coefficients after sampleRate changes.
    void updateSmoothingCoefficients();
};

void BiQuadFilter::Impl::computeTargetCoefficients(float frequency, float q, FilterType type)
{
    // Clamp to just below Nyquist — above fs/2 the math aliases silently.
    const float nyquist = static_cast<float>(sampleRate) * 0.5f;
    frequency = std::min(frequency, nyquist - 1.0f);
    frequency = std::max(frequency, 1.0f);  // avoid division-by-zero at DC

    float w0 = 2.0f * 3.14159265358979323846f * frequency / static_cast<float>(sampleRate);
    float cosW0 = std::cos(w0);
    float sinW0 = std::sin(w0);
    float alpha = sinW0 / (2.0f * q);

    float b0, b1, b2, a0, a1, a2;

    switch (type) {
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

    // Normalize so that a0 == 1.0, then store as targets
    targetB0 = b0 / a0; targetB1 = b1 / a0; targetB2 = b2 / a0;
    targetA1 = a1 / a0; targetA2 = a2 / a0;
}

void BiQuadFilter::Impl::calculateCoefficients()
{
    // Read raw (target) values from the parameter objects.  This is the
    // entry point for change callbacks — any external system that writes
    // a parameter value (automation, MIDI CC, preset load) lands here and
    // immediately updates the coefficient targets.  The running coefficients
    // will smooth toward these targets in the next process() call.
    computeTargetCoefficients(
        paramFrequency->getValue(),
        paramQ->getValue(),
        static_cast<FilterType>(paramFilterType->getSelectedIndex())
    );
}

void BiQuadFilter::Impl::updateSmoothingCoefficients()
{
    constexpr float kCoeffSmoothingTimeSeconds = 0.010f;  // 10 ms
    constexpr float kParamSmoothingTimeSeconds = 0.010f;  // 10 ms

    coeffSmoothCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * kCoeffSmoothingTimeSeconds));
    paramSmoothCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * kParamSmoothingTimeSeconds));
}

BiQuadFilter::BiQuadFilter()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "BiQuadFilter_" + std::to_string(++instanceCounter);

    // --- Create IParameter objects with smoothing enabled ---
    m_impl->paramFilterType = std::make_unique<EnumParameter>(
        "filterType",
        std::vector<std::string>{"LowPass", "HighPass", "BandPass", "Notch",
                                 "AllPass", "PeakingEQ", "LowShelf", "HighShelf"},
        0  // default: LowPass
    );
    m_impl->paramFrequency = std::make_unique<FloatParameter>(
        "frequency", 1000.0f, 20.0f, 20000.0f
    );
    m_impl->paramFrequency->enableSmoothing(true, 0.010f);  // 10 ms

    m_impl->paramQ = std::make_unique<FloatParameter>(
        "q", 0.707f, 0.1f, 10.0f
    );
    m_impl->paramQ->enableSmoothing(true, 0.010f);

    m_impl->paramGain = std::make_unique<FloatParameter>(
        "gain", 0.0f, -24.0f, 24.0f
    );
    m_impl->paramGain->enableSmoothing(true, 0.010f);

    // --- Wire change callbacks ---
    // Any external write (automation lane, MIDI CC, preset load) calls
    // setValue() on the parameter, which fires this callback synchronously.
    // The callback recomputes target coefficients from the new raw value
    // so that the next process() block begins smoothing toward them.
    auto* impl = m_impl.get();
    m_impl->paramFrequency->setChangeCallback(
        [impl](float, float) { impl->calculateCoefficients(); }
    );
    m_impl->paramQ->setChangeCallback(
        [impl](float, float) { impl->calculateCoefficients(); }
    );
    m_impl->paramGain->setChangeCallback(
        [impl](float, float) { impl->calculateCoefficients(); }
    );
    m_impl->paramFilterType->setChangeCallback(
        [impl](size_t, size_t) { impl->calculateCoefficients(); }
    );

    // --- Initial coefficient computation ---
    m_impl->updateSmoothingCoefficients();
    m_impl->calculateCoefficients();

    // Seed running coefficients = targets so the first block doesn't
    // ramp from zero.
    m_impl->b0 = m_impl->targetB0; m_impl->b1 = m_impl->targetB1; m_impl->b2 = m_impl->targetB2;
    m_impl->a1 = m_impl->targetA1; m_impl->a2 = m_impl->targetA2;

    // Seed cached smoothed param values to match defaults
    m_impl->smoothedFrequency = m_impl->paramFrequency->getValue();
    m_impl->smoothedQ         = m_impl->paramQ->getValue();
    m_impl->smoothedGain      = m_impl->paramGain->getValue();
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

    // --- Stage 1: Advance parameter smoothers for this block ----------
    // We maintain our own cached smoothed values (smoothedFrequency, etc.)
    // using the pre-cached paramSmoothCoeff.  This avoids calling
    // FloatParameter::getSmoothedValue() per sample (which recomputes
    // exp() every call).  We advance numFrames steps so the smoother
    // tracks at sample rate, then read the FloatParameter's raw target
    // via getValue() as the destination.
    //
    // All locals — zero allocation.
    {
        const float psc       = m_impl->paramSmoothCoeff;
        const float targetFreq = m_impl->paramFrequency->getValue();
        const float targetQ    = m_impl->paramQ->getValue();
        const float targetGain = m_impl->paramGain->getValue();

        float sf = m_impl->smoothedFrequency;
        float sq = m_impl->smoothedQ;
        float sg = m_impl->smoothedGain;

        for (std::uint32_t i = 0; i < numFrames; ++i) {
            sf = targetFreq + psc * (sf - targetFreq);
            sq = targetQ    + psc * (sq - targetQ);
            sg = targetGain + psc * (sg - targetGain);
        }

        m_impl->smoothedFrequency = sf;
        m_impl->smoothedQ         = sq;
        m_impl->smoothedGain      = sg;

        // Sync the FloatParameter internal smoothed state so that any
        // external call to getSmoothedValue() stays consistent with what
        // we just computed.  One call per block is fine — we only pay
        // one exp() here, not numFrames.
        m_impl->paramFrequency->getSmoothedValue(static_cast<float>(m_impl->sampleRate));
        m_impl->paramQ->getSmoothedValue(static_cast<float>(m_impl->sampleRate));
        m_impl->paramGain->getSmoothedValue(static_cast<float>(m_impl->sampleRate));
    }

    // --- Stage 2: Recompute target coefficients from smoothed params ---
    // One sin + cos per block — negligible vs. numFrames of TDF-II.
    m_impl->computeTargetCoefficients(
        m_impl->smoothedFrequency,
        m_impl->smoothedQ,
        static_cast<FilterType>(m_impl->paramFilterType->getSelectedIndex())
    );

    // --- Stage 3: Per-sample loop — coefficient smoothing + TDF-II ----
    // Everything below is stack locals.  No heap access after this point.
    //
    // Coefficient smoothing (one-pole toward targets):
    //   coeff = coeff + csc * (target - coeff)
    //         = target + csc * (coeff - target)   [equivalent, fewer ops]
    //
    // TDF-II:
    //   y[n] = b0 * x[n] + z1
    //   z1   = b1 * x[n] - a1 * y[n] + z2
    //   z2   = b2 * x[n] - a2 * y[n]

    const float csc  = m_impl->coeffSmoothCoeff;
    const float tb0  = m_impl->targetB0, tb1 = m_impl->targetB1, tb2 = m_impl->targetB2;
    const float ta1  = m_impl->targetA1, ta2 = m_impl->targetA2;

    // Snapshot block-start coefficients — both channels must begin from
    // the same point so their smoothing trajectories are identical.
    const float startB0 = m_impl->b0, startB1 = m_impl->b1, startB2 = m_impl->b2;
    const float startA1 = m_impl->a1, startA2 = m_impl->a2;

    float b0 = startB0, b1 = startB1, b2 = startB2;
    float a1 = startA1, a2 = startA2;
    float z1L = m_impl->z1L, z2L = m_impl->z2L;

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        // Smooth running coefficients toward targets
        b0 = tb0 + csc * (b0 - tb0);
        b1 = tb1 + csc * (b1 - tb1);
        b2 = tb2 + csc * (b2 - tb2);
        a1 = ta1 + csc * (a1 - ta1);
        a2 = ta2 + csc * (a2 - ta2);

        // TDF-II
        float x = inputBuffer[i * numChannels];
        float y = b0 * x + z1L;
        z1L = b1 * x - a1 * y + z2L;
        z2L = b2 * x - a2 * y;
        outputBuffer[i * numChannels] = y;
    }

    // Persist end-of-block smoothed coefficients — next block resumes here.
    m_impl->b0 = b0; m_impl->b1 = b1; m_impl->b2 = b2;
    m_impl->a1 = a1; m_impl->a2 = a2;
    m_impl->z1L = z1L; m_impl->z2L = z2L;

    if (numChannels > 1) {
        float z1R = m_impl->z1R, z2R = m_impl->z2R;
        // Restart from the same block-start coefficients the left channel used
        b0 = startB0; b1 = startB1; b2 = startB2;
        a1 = startA1; a2 = startA2;

        for (std::uint32_t i = 0; i < numFrames; ++i) {
            b0 = tb0 + csc * (b0 - tb0);
            b1 = tb1 + csc * (b1 - tb1);
            b2 = tb2 + csc * (b2 - tb2);
            a1 = ta1 + csc * (a1 - ta1);
            a2 = ta2 + csc * (a2 - ta2);

            float x = inputBuffer[i * numChannels + 1];
            float y = b0 * x + z1R;
            z1R = b1 * x - a1 * y + z2R;
            z2R = b2 * x - a2 * y;
            outputBuffer[i * numChannels + 1] = y;
        }

        m_impl->z1R = z1R; m_impl->z2R = z2R;
    }
}

void BiQuadFilter::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
    m_impl->updateSmoothingCoefficients();
    m_impl->calculateCoefficients();

    // Snap running coefficients to targets on rate change to avoid
    // a multi-second ramp if the old smoothing state was far off.
    m_impl->b0 = m_impl->targetB0; m_impl->b1 = m_impl->targetB1; m_impl->b2 = m_impl->targetB2;
    m_impl->a1 = m_impl->targetA1; m_impl->a2 = m_impl->targetA2;

    // Re-seed cached smoothed params
    m_impl->smoothedFrequency = m_impl->paramFrequency->getValue();
    m_impl->smoothedQ         = m_impl->paramQ->getValue();
    m_impl->smoothedGain      = m_impl->paramGain->getValue();
}

void BiQuadFilter::reset()
{
    // Clear TDF-II delay state
    m_impl->z1L = m_impl->z2L = 0.0f;
    m_impl->z1R = m_impl->z2R = 0.0f;

    // Snap running coefficients to current targets — no ramp after reset
    m_impl->b0 = m_impl->targetB0; m_impl->b1 = m_impl->targetB1; m_impl->b2 = m_impl->targetB2;
    m_impl->a1 = m_impl->targetA1; m_impl->a2 = m_impl->targetA2;
}

std::string BiQuadFilter::getNodeId() const { return m_impl->nodeId; }
std::string BiQuadFilter::getTypeName() const { return "BiQuadFilter"; }
std::uint32_t BiQuadFilter::getNumInputChannels() const { return 2; }
std::uint32_t BiQuadFilter::getNumOutputChannels() const { return 2; }
bool BiQuadFilter::isBypassed() const { return m_impl->bypassed; }
void BiQuadFilter::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

// Parameter setters delegate entirely to the IParameter objects.
// setValue() / setSelectedIndex() fire the change callbacks registered in
// the constructor, which call calculateCoefficients() automatically.
// This is the single entry point for automation, MIDI CC, preset load, etc.

void BiQuadFilter::setFilterType(FilterType type)
{
    m_impl->paramFilterType->setSelectedIndex(static_cast<size_t>(type));
}
BiQuadFilter::FilterType BiQuadFilter::getFilterType() const
{
    return static_cast<FilterType>(m_impl->paramFilterType->getSelectedIndex());
}

void BiQuadFilter::setFrequency(float frequencyHz)
{
    m_impl->paramFrequency->setValue(frequencyHz);
}
float BiQuadFilter::getFrequency() const { return m_impl->paramFrequency->getValue(); }

void BiQuadFilter::setQ(float q)
{
    m_impl->paramQ->setValue(q);
}
float BiQuadFilter::getQ() const { return m_impl->paramQ->getValue(); }

void BiQuadFilter::setGain(float gainDb)
{
    m_impl->paramGain->setValue(gainDb);
}
float BiQuadFilter::getGain() const { return m_impl->paramGain->getValue(); }

} // namespace nap
