#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/BiQuadFilter.h"
#include <cmath>
#include <vector>
#include <numeric>

namespace nap { namespace test {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Generate a mono impulse response of length `numSamples` through `filter`.
static std::vector<float> captureImpulseResponse(BiQuadFilter& filter, uint32_t numSamples)
{
    std::vector<float> input(numSamples, 0.0f);
    std::vector<float> output(numSamples, 0.0f);
    input[0] = 1.0f;  // unit impulse

    filter.reset();
    filter.process(input.data(), output.data(), numSamples, 1);
    return output;
}

// Compute magnitude (linear) of the frequency response at a single bin
// by evaluating the DFT at that frequency from an impulse response.
static float magnitudeAtFrequency(const std::vector<float>& ir, double freqHz, double sampleRate)
{
    double sumReal = 0.0, sumImag = 0.0;
    const double w = 2.0 * 3.14159265358979323846 * freqHz / sampleRate;
    for (size_t n = 0; n < ir.size(); ++n) {
        sumReal += ir[n] * std::cos(w * n);
        sumImag -= ir[n] * std::sin(w * n);
    }
    return static_cast<float>(std::sqrt(sumReal * sumReal + sumImag * sumImag));
}

static float magnitudeToDb(float linear)
{
    return 20.0f * std::log10f(linear + 1e-12f);
}

constexpr uint32_t IR_LENGTH   = 4096;
constexpr double   SAMPLE_RATE = 44100.0;

// ---------------------------------------------------------------------------
// Basic interface tests
// ---------------------------------------------------------------------------

TEST(BiQuadFilterTest, DefaultFilterTypeIsLowPass) {
    BiQuadFilter filter;
    EXPECT_EQ(filter.getFilterType(), BiQuadFilter::FilterType::LowPass);
}

TEST(BiQuadFilterTest, CanSetFilterType) {
    BiQuadFilter filter;
    filter.setFilterType(BiQuadFilter::FilterType::HighPass);
    EXPECT_EQ(filter.getFilterType(), BiQuadFilter::FilterType::HighPass);
}

TEST(BiQuadFilterTest, CanSetFrequency) {
    BiQuadFilter filter;
    filter.setFrequency(2000.0f);
    EXPECT_FLOAT_EQ(filter.getFrequency(), 2000.0f);
}

TEST(BiQuadFilterTest, CanSetQ) {
    BiQuadFilter filter;
    filter.setQ(1.5f);
    EXPECT_FLOAT_EQ(filter.getQ(), 1.5f);
}

TEST(BiQuadFilterTest, HasCorrectTypeName) {
    BiQuadFilter filter;
    EXPECT_EQ(filter.getTypeName(), "BiQuadFilter");
}

// ---------------------------------------------------------------------------
// Frequency response verification — these prove the filter actually filters
// ---------------------------------------------------------------------------

TEST(BiQuadFilterTest, LowPass_AttenuatesAboveCutoff) {
    BiQuadFilter filter;
    filter.prepare(SAMPLE_RATE, IR_LENGTH);
    filter.setFilterType(BiQuadFilter::FilterType::LowPass);
    filter.setFrequency(1000.0f);
    filter.setQ(0.707f);

    auto ir = captureImpulseResponse(filter, IR_LENGTH);

    // Well below cutoff: should be near 0 dB
    float magPassband = magnitudeToDb(magnitudeAtFrequency(ir, 100.0, SAMPLE_RATE));
    EXPECT_GT(magPassband, -3.0f) << "Passband (100 Hz) should be near unity";

    // Well above cutoff: should be heavily attenuated
    float magStopband = magnitudeToDb(magnitudeAtFrequency(ir, 10000.0, SAMPLE_RATE));
    EXPECT_LT(magStopband, -20.0f) << "Stopband (10 kHz) should be attenuated > 20 dB";
}

TEST(BiQuadFilterTest, HighPass_AttenuatesBelowCutoff) {
    BiQuadFilter filter;
    filter.prepare(SAMPLE_RATE, IR_LENGTH);
    filter.setFilterType(BiQuadFilter::FilterType::HighPass);
    filter.setFrequency(5000.0f);
    filter.setQ(0.707f);

    auto ir = captureImpulseResponse(filter, IR_LENGTH);

    float magStopband = magnitudeToDb(magnitudeAtFrequency(ir, 200.0, SAMPLE_RATE));
    EXPECT_LT(magStopband, -20.0f) << "Stopband (200 Hz) should be attenuated > 20 dB";

    float magPassband = magnitudeToDb(magnitudeAtFrequency(ir, 15000.0, SAMPLE_RATE));
    EXPECT_GT(magPassband, -3.0f) << "Passband (15 kHz) should be near unity";
}

TEST(BiQuadFilterTest, BandPass_AttenuatesOutsideBand) {
    BiQuadFilter filter;
    filter.prepare(SAMPLE_RATE, IR_LENGTH);
    filter.setFilterType(BiQuadFilter::FilterType::BandPass);
    filter.setFrequency(2000.0f);
    filter.setQ(2.0f);  // narrow-ish band

    auto ir = captureImpulseResponse(filter, IR_LENGTH);

    // Center frequency should pass
    float magCenter = magnitudeToDb(magnitudeAtFrequency(ir, 2000.0, SAMPLE_RATE));
    EXPECT_GT(magCenter, -6.0f) << "Center (2 kHz) should be near peak";

    // Far below and above should be attenuated
    float magLow = magnitudeToDb(magnitudeAtFrequency(ir, 100.0, SAMPLE_RATE));
    EXPECT_LT(magLow, -12.0f) << "Below band (100 Hz) should be attenuated";

    float magHigh = magnitudeToDb(magnitudeAtFrequency(ir, 15000.0, SAMPLE_RATE));
    EXPECT_LT(magHigh, -12.0f) << "Above band (15 kHz) should be attenuated";
}

TEST(BiQuadFilterTest, ResetClearsDelayState) {
    BiQuadFilter filter;
    filter.prepare(SAMPLE_RATE, 64);
    filter.setFilterType(BiQuadFilter::FilterType::LowPass);
    filter.setFrequency(1000.0f);

    // Run some signal through to populate state
    std::vector<float> input(64, 0.5f);
    std::vector<float> output(64);
    filter.process(input.data(), output.data(), 64, 1);

    // After reset, processing a zero input should produce zero output
    filter.reset();
    std::vector<float> zeros(64, 0.0f);
    std::vector<float> afterReset(64, 99.0f);  // sentinel
    filter.process(zeros.data(), afterReset.data(), 64, 1);

    for (uint32_t i = 0; i < 64; ++i) {
        EXPECT_FLOAT_EQ(afterReset[i], 0.0f) << "Sample " << i << " should be zero after reset";
    }
}

TEST(BiQuadFilterTest, StereoProcessingMatchesMonoPerChannel) {
    // Process two independent mono signals, then compare against
    // stereo interleaved processing. Results must be identical.
    BiQuadFilter monoL, monoR, stereo;
    const uint32_t N = 256;

    auto setup = [&](BiQuadFilter& f) {
        f.prepare(SAMPLE_RATE, N);
        f.setFilterType(BiQuadFilter::FilterType::LowPass);
        f.setFrequency(800.0f);
        f.setQ(0.707f);
        f.reset();
    };
    setup(monoL); setup(monoR); setup(stereo);

    std::vector<float> sigL(N), sigR(N);
    for (uint32_t i = 0; i < N; ++i) {
        sigL[i] = std::sin(2.0f * 3.14159265358979f * 440.0f * i / SAMPLE_RATE);
        sigR[i] = std::sin(2.0f * 3.14159265358979f * 880.0f * i / SAMPLE_RATE);
    }

    // Interleaved stereo buffer
    std::vector<float> stereoIn(N * 2), stereoOut(N * 2);
    for (uint32_t i = 0; i < N; ++i) {
        stereoIn[i * 2]     = sigL[i];
        stereoIn[i * 2 + 1] = sigR[i];
    }

    std::vector<float> outL(N), outR(N);
    monoL.process(sigL.data(), outL.data(), N, 1);
    monoR.process(sigR.data(), outR.data(), N, 1);
    stereo.process(stereoIn.data(), stereoOut.data(), N, 2);

    for (uint32_t i = 0; i < N; ++i) {
        EXPECT_NEAR(stereoOut[i * 2],     outL[i], 1e-6f)
            << "Left channel mismatch at frame " << i;
        EXPECT_NEAR(stereoOut[i * 2 + 1], outR[i], 1e-6f)
            << "Right channel mismatch at frame " << i;
    }
}

TEST(BiQuadFilterTest, BypassPassesSignalThrough) {
    BiQuadFilter filter;
    filter.prepare(SAMPLE_RATE, 64);
    filter.setBypassed(true);

    std::vector<float> input(64);
    for (uint32_t i = 0; i < 64; ++i) input[i] = static_cast<float>(i) * 0.01f;

    std::vector<float> output(64);
    filter.process(input.data(), output.data(), 64, 1);

    for (uint32_t i = 0; i < 64; ++i) {
        EXPECT_FLOAT_EQ(output[i], input[i]);
    }
}

}} // namespace nap::test
