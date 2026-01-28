#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/BiQuadFilter.h"
#include <cmath>
#include <vector>
#include <numeric>

namespace nap { namespace test {

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

// Helper function to calculate RMS (Root Mean Square) amplitude
static float calculateRMS(const float* buffer, size_t size) {
    float sumSquares = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        sumSquares += buffer[i] * buffer[i];
    }
    return std::sqrt(sumSquares / size);
}

// Generate sine wave test signal
static void generateSine(float* buffer, size_t numFrames, float frequency, double sampleRate) {
    for (size_t i = 0; i < numFrames; ++i) {
        buffer[i * 2] = std::sin(2.0 * M_PI * frequency * i / sampleRate);      // Left
        buffer[i * 2 + 1] = std::sin(2.0 * M_PI * frequency * i / sampleRate);  // Right
    }
}

TEST(BiQuadFilterTest, LowPassAttenuatesHighFrequencies) {
    BiQuadFilter filter;
    filter.setFilterType(BiQuadFilter::FilterType::LowPass);
    filter.setFrequency(1000.0f);  // Cutoff at 1kHz
    filter.setQ(0.707f);           // Butterworth response
    filter.prepare(44100.0, 512);

    const size_t numFrames = 2048;
    std::vector<float> inputLow(numFrames * 2);
    std::vector<float> outputLow(numFrames * 2);
    std::vector<float> inputHigh(numFrames * 2);
    std::vector<float> outputHigh(numFrames * 2);

    // Test with 500Hz (below cutoff - should pass)
    generateSine(inputLow.data(), numFrames, 500.0f, 44100.0);
    filter.reset();
    filter.process(inputLow.data(), outputLow.data(), numFrames, 2);

    // Test with 5kHz (above cutoff - should attenuate)
    generateSine(inputHigh.data(), numFrames, 5000.0f, 44100.0);
    filter.reset();
    filter.process(inputHigh.data(), outputHigh.data(), numFrames, 2);

    // Calculate RMS amplitudes (use second half to avoid transients)
    float rmsLowInput = calculateRMS(inputLow.data() + numFrames, numFrames);
    float rmsLowOutput = calculateRMS(outputLow.data() + numFrames, numFrames);
    float rmsHighInput = calculateRMS(inputHigh.data() + numFrames, numFrames);
    float rmsHighOutput = calculateRMS(outputHigh.data() + numFrames, numFrames);

    float gainLow = rmsLowOutput / rmsLowInput;
    float gainHigh = rmsHighOutput / rmsHighInput;

    // Low frequency should pass with minimal attenuation (> 0.9)
    EXPECT_GT(gainLow, 0.9f);

    // High frequency should be attenuated significantly (< 0.3)
    EXPECT_LT(gainHigh, 0.3f);

    // Low frequency gain should be much higher than high frequency gain
    EXPECT_GT(gainLow, gainHigh * 2.0f);
}

TEST(BiQuadFilterTest, HighPassAttenuatesLowFrequencies) {
    BiQuadFilter filter;
    filter.setFilterType(BiQuadFilter::FilterType::HighPass);
    filter.setFrequency(1000.0f);  // Cutoff at 1kHz
    filter.setQ(0.707f);
    filter.prepare(44100.0, 512);

    const size_t numFrames = 2048;
    std::vector<float> inputLow(numFrames * 2);
    std::vector<float> outputLow(numFrames * 2);
    std::vector<float> inputHigh(numFrames * 2);
    std::vector<float> outputHigh(numFrames * 2);

    // Test with 200Hz (below cutoff - should attenuate)
    generateSine(inputLow.data(), numFrames, 200.0f, 44100.0);
    filter.reset();
    filter.process(inputLow.data(), outputLow.data(), numFrames, 2);

    // Test with 5kHz (above cutoff - should pass)
    generateSine(inputHigh.data(), numFrames, 5000.0f, 44100.0);
    filter.reset();
    filter.process(inputHigh.data(), outputHigh.data(), numFrames, 2);

    float rmsLowInput = calculateRMS(inputLow.data() + numFrames, numFrames);
    float rmsLowOutput = calculateRMS(outputLow.data() + numFrames, numFrames);
    float rmsHighInput = calculateRMS(inputHigh.data() + numFrames, numFrames);
    float rmsHighOutput = calculateRMS(outputHigh.data() + numFrames, numFrames);

    float gainLow = rmsLowOutput / rmsLowInput;
    float gainHigh = rmsHighOutput / rmsHighInput;

    // Low frequency should be attenuated (< 0.3)
    EXPECT_LT(gainLow, 0.3f);

    // High frequency should pass with minimal attenuation (> 0.9)
    EXPECT_GT(gainHigh, 0.9f);

    // High frequency gain should be much higher than low frequency gain
    EXPECT_GT(gainHigh, gainLow * 2.0f);
}

TEST(BiQuadFilterTest, BandPassHasCorrectResponse) {
    BiQuadFilter filter;
    filter.setFilterType(BiQuadFilter::FilterType::BandPass);
    filter.setFrequency(1000.0f);  // Center at 1kHz
    filter.setQ(5.0f);             // Narrow band
    filter.prepare(44100.0, 512);

    const size_t numFrames = 2048;
    std::vector<float> inputLow(numFrames * 2);
    std::vector<float> outputLow(numFrames * 2);
    std::vector<float> inputCenter(numFrames * 2);
    std::vector<float> outputCenter(numFrames * 2);
    std::vector<float> inputHigh(numFrames * 2);
    std::vector<float> outputHigh(numFrames * 2);

    // Test with 200Hz (below center - should attenuate)
    generateSine(inputLow.data(), numFrames, 200.0f, 44100.0);
    filter.reset();
    filter.process(inputLow.data(), outputLow.data(), numFrames, 2);

    // Test with 1kHz (at center - should pass)
    generateSine(inputCenter.data(), numFrames, 1000.0f, 44100.0);
    filter.reset();
    filter.process(inputCenter.data(), outputCenter.data(), numFrames, 2);

    // Test with 5kHz (above center - should attenuate)
    generateSine(inputHigh.data(), numFrames, 5000.0f, 44100.0);
    filter.reset();
    filter.process(inputHigh.data(), outputHigh.data(), numFrames, 2);

    float rmsLowOutput = calculateRMS(outputLow.data() + numFrames, numFrames);
    float rmsCenterOutput = calculateRMS(outputCenter.data() + numFrames, numFrames);
    float rmsHighOutput = calculateRMS(outputHigh.data() + numFrames, numFrames);

    // Center frequency should have highest output
    EXPECT_GT(rmsCenterOutput, rmsLowOutput * 2.0f);
    EXPECT_GT(rmsCenterOutput, rmsHighOutput * 2.0f);
}

TEST(BiQuadFilterTest, BypassDoesNotModifySignal) {
    BiQuadFilter filter;
    filter.setFilterType(BiQuadFilter::FilterType::LowPass);
    filter.setFrequency(100.0f);  // Very aggressive filter
    filter.setQ(0.707f);
    filter.prepare(44100.0, 512);
    filter.setBypassed(true);

    const size_t numFrames = 512;
    std::vector<float> input(numFrames * 2);
    std::vector<float> output(numFrames * 2);

    // Generate high frequency signal
    generateSine(input.data(), numFrames, 10000.0f, 44100.0);

    filter.process(input.data(), output.data(), numFrames, 2);

    // Output should be identical to input when bypassed
    for (size_t i = 0; i < numFrames * 2; ++i) {
        EXPECT_FLOAT_EQ(output[i], input[i]);
    }
}

TEST(BiQuadFilterTest, ResetClearsState) {
    BiQuadFilter filter;
    filter.prepare(44100.0, 512);

    const size_t numFrames = 512;
    std::vector<float> input(numFrames * 2, 1.0f);  // DC signal
    std::vector<float> output1(numFrames * 2);
    std::vector<float> output2(numFrames * 2);

    // Process once
    filter.process(input.data(), output1.data(), numFrames, 2);

    // Reset and process again
    filter.reset();
    filter.process(input.data(), output2.data(), numFrames, 2);

    // First few samples should be identical after reset
    EXPECT_FLOAT_EQ(output1[0], output2[0]);
    EXPECT_FLOAT_EQ(output1[1], output2[1]);
}

}} // namespace nap::test
