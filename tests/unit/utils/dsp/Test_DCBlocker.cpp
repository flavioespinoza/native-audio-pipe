#include <gtest/gtest.h>
#include "utils/dsp/DCBlocker.h"
#include <cmath>
#include <numeric>

namespace nap {
namespace test {

class DCBlockerTest : public ::testing::Test {
protected:
    void SetUp() override {
        blocker = std::make_unique<DCBlocker>();
    }

    std::unique_ptr<DCBlocker> blocker;
};

TEST_F(DCBlockerTest, DefaultConstruction) {
    EXPECT_NEAR(blocker->getR(), 0.995f, 0.001f);
}

TEST_F(DCBlockerTest, ConstructWithR) {
    DCBlocker customBlocker(0.999f);
    EXPECT_NEAR(customBlocker.getR(), 0.999f, 0.001f);
}

TEST_F(DCBlockerTest, SetR) {
    blocker->setR(0.99f);
    EXPECT_NEAR(blocker->getR(), 0.99f, 0.001f);
}

TEST_F(DCBlockerTest, SetCutoffFrequency) {
    blocker->setCutoffFrequency(20.0f, 44100.0f);

    float cutoff = blocker->getCutoffFrequency(44100.0f);
    EXPECT_NEAR(cutoff, 20.0f, 5.0f);  // Allow some tolerance
}

TEST_F(DCBlockerTest, RemovesDCOffset) {
    // Create signal with DC offset
    const size_t numSamples = 10000;
    std::vector<float> signal(numSamples);
    const float dcOffset = 0.5f;

    for (size_t i = 0; i < numSamples; ++i) {
        signal[i] = dcOffset + 0.1f * std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * i / 44100.0f);
    }

    // Process through DC blocker
    blocker->process(signal.data(), numSamples);

    // Calculate mean of latter half (after settling)
    float mean = 0.0f;
    for (size_t i = numSamples / 2; i < numSamples; ++i) {
        mean += signal[i];
    }
    mean /= (numSamples / 2);

    // Mean should be close to zero (DC removed)
    EXPECT_NEAR(mean, 0.0f, 0.05f);
}

TEST_F(DCBlockerTest, PreservesACContent) {
    // Pure sine wave with no DC should pass through mostly unchanged
    const size_t numSamples = 4410;  // 100ms at 44.1kHz
    std::vector<float> original(numSamples);
    std::vector<float> processed(numSamples);

    for (size_t i = 0; i < numSamples; ++i) {
        original[i] = std::sin(2.0f * static_cast<float>(M_PI) * 1000.0f * i / 44100.0f);
    }

    blocker->process(original.data(), processed.data(), numSamples);

    // Calculate RMS of both signals (after settling)
    float originalRMS = 0.0f;
    float processedRMS = 0.0f;
    for (size_t i = numSamples / 2; i < numSamples; ++i) {
        originalRMS += original[i] * original[i];
        processedRMS += processed[i] * processed[i];
    }

    // RMS should be similar (signal preserved)
    originalRMS = std::sqrt(originalRMS / (numSamples / 2));
    processedRMS = std::sqrt(processedRMS / (numSamples / 2));
    EXPECT_NEAR(processedRMS, originalRMS, 0.1f);
}

TEST_F(DCBlockerTest, Reset) {
    // Process some samples
    float sample = blocker->process(1.0f);
    sample = blocker->process(1.0f);
    (void)sample;

    // Reset and verify state cleared
    blocker->reset();

    // First sample after reset should behave like fresh start
    float firstAfterReset = blocker->process(0.0f);
    EXPECT_FLOAT_EQ(firstAfterReset, 0.0f);
}

TEST_F(DCBlockerTest, ProcessSingleSample) {
    float input = 1.0f;
    float output = blocker->process(input);

    // First sample: y[0] = x[0] - 0 + R * 0 = x[0]
    EXPECT_FLOAT_EQ(output, 1.0f);

    // Second sample with same input
    output = blocker->process(input);
    // y[1] = 1 - 1 + R * 1 = R
    EXPECT_NEAR(output, blocker->getR(), 0.001f);
}

TEST_F(DCBlockerTest, CalculateR) {
    // Higher cutoff = lower R
    float r1 = DCBlocker::calculateR(10.0f, 44100.0f);
    float r2 = DCBlocker::calculateR(100.0f, 44100.0f);

    EXPECT_GT(r1, r2);

    // R should be clamped to valid range
    EXPECT_GE(r1, 0.9f);
    EXPECT_LE(r1, 0.9999f);
}

// Multi-channel tests
class MultiChannelDCBlockerTest : public ::testing::Test {
protected:
    void SetUp() override {
        blocker = std::make_unique<MultiChannelDCBlocker>(2);
    }

    std::unique_ptr<MultiChannelDCBlocker> blocker;
};

TEST_F(MultiChannelDCBlockerTest, Construction) {
    EXPECT_EQ(blocker->getNumChannels(), 2u);
}

TEST_F(MultiChannelDCBlockerTest, SetNumChannels) {
    blocker->setNumChannels(8);
    EXPECT_EQ(blocker->getNumChannels(), 8u);
}

TEST_F(MultiChannelDCBlockerTest, ProcessInterleaved) {
    const size_t numFrames = 1000;
    const size_t numChannels = 2;
    std::vector<float> buffer(numFrames * numChannels);

    // Fill with DC offset on both channels
    for (size_t i = 0; i < buffer.size(); ++i) {
        buffer[i] = 0.5f;
    }

    blocker->processInterleaved(buffer.data(), numFrames);

    // Check that DC is being removed (after settling)
    float sum = 0.0f;
    for (size_t i = numFrames / 2 * numChannels; i < buffer.size(); ++i) {
        sum += std::abs(buffer[i]);
    }
    float avgMagnitude = sum / (numFrames / 2 * numChannels);
    EXPECT_LT(avgMagnitude, 0.1f);
}

TEST_F(MultiChannelDCBlockerTest, ProcessNonInterleaved) {
    const size_t numFrames = 1000;
    std::vector<float> ch1(numFrames, 0.5f);
    std::vector<float> ch2(numFrames, 0.3f);
    float* channels[] = {ch1.data(), ch2.data()};

    blocker->processNonInterleaved(channels, numFrames);

    // Check DC removal on both channels
    float sum1 = 0.0f, sum2 = 0.0f;
    for (size_t i = numFrames / 2; i < numFrames; ++i) {
        sum1 += std::abs(ch1[i]);
        sum2 += std::abs(ch2[i]);
    }
    EXPECT_LT(sum1 / (numFrames / 2), 0.1f);
    EXPECT_LT(sum2 / (numFrames / 2), 0.1f);
}

TEST_F(MultiChannelDCBlockerTest, Reset) {
    blocker->processInterleaved(std::vector<float>(100, 1.0f).data(), 50);
    blocker->reset();
    // Should not crash
    SUCCEED();
}

TEST_F(MultiChannelDCBlockerTest, SetCutoffFrequency) {
    blocker->setCutoffFrequency(30.0f, 48000.0f);
    // Should not crash
    SUCCEED();
}

} // namespace test
} // namespace nap
