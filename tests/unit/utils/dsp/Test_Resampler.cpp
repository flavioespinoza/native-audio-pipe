#include <gtest/gtest.h>
#include "utils/dsp/Resampler.h"
#include <cmath>

namespace nap {
namespace test {

class ResamplerTest : public ::testing::Test {
protected:
    void SetUp() override {
        resampler = std::make_unique<Resampler>();
    }

    std::unique_ptr<Resampler> resampler;
};

TEST_F(ResamplerTest, DefaultConstruction) {
    EXPECT_EQ(resampler->getQuality(), ResamplerQuality::Medium);
}

TEST_F(ResamplerTest, ConstructWithQuality) {
    Resampler highQuality(ResamplerQuality::High);
    EXPECT_EQ(highQuality.getQuality(), ResamplerQuality::High);
}

TEST_F(ResamplerTest, SetQuality) {
    resampler->setQuality(ResamplerQuality::Best);
    EXPECT_EQ(resampler->getQuality(), ResamplerQuality::Best);
}

TEST_F(ResamplerTest, UpsampleDoubleRate) {
    // Create a simple test signal
    std::vector<float> input(100);
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = std::sin(2.0f * static_cast<float>(M_PI) * i / 10.0f);
    }

    auto output = resampler->resample(input, 44100.0, 88200.0);

    // Output should be approximately twice as long
    EXPECT_NEAR(output.size(), input.size() * 2, 2);
}

TEST_F(ResamplerTest, DownsampleHalfRate) {
    std::vector<float> input(100);
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = std::sin(2.0f * static_cast<float>(M_PI) * i / 20.0f);
    }

    auto output = resampler->resample(input, 88200.0, 44100.0);

    // Output should be approximately half as long
    EXPECT_NEAR(output.size(), input.size() / 2, 2);
}

TEST_F(ResamplerTest, NoChange) {
    std::vector<float> input(100);
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = static_cast<float>(i) / 100.0f;
    }

    auto output = resampler->resample(input, 44100.0, 44100.0);

    EXPECT_EQ(output.size(), input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_NEAR(output[i], input[i], 0.01f);
    }
}

TEST_F(ResamplerTest, EmptyInput) {
    std::vector<float> input;
    auto output = resampler->resample(input, 44100.0, 48000.0);
    EXPECT_TRUE(output.empty());
}

TEST_F(ResamplerTest, GetOutputSize) {
    size_t outputSize = Resampler::getOutputSize(100, 2.0);
    EXPECT_EQ(outputSize, 200u);

    outputSize = Resampler::getOutputSize(100, 0.5);
    EXPECT_EQ(outputSize, 50u);
}

TEST_F(ResamplerTest, GetInputSize) {
    size_t inputSize = Resampler::getInputSize(200, 2.0);
    EXPECT_EQ(inputSize, 100u);

    inputSize = Resampler::getInputSize(50, 0.5);
    EXPECT_EQ(inputSize, 100u);
}

TEST_F(ResamplerTest, GetLatency) {
    resampler->setQuality(ResamplerQuality::Fast);
    size_t fastLatency = resampler->getLatency();

    resampler->setQuality(ResamplerQuality::Best);
    size_t bestLatency = resampler->getLatency();

    // Higher quality should have more latency
    EXPECT_GT(bestLatency, fastLatency);
}

TEST_F(ResamplerTest, IntegerUpsample) {
    std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f};
    auto output = Resampler::upsample(input, 2);

    EXPECT_EQ(output.size(), 8u);
    EXPECT_FLOAT_EQ(output[0], 1.0f);
    EXPECT_FLOAT_EQ(output[2], 2.0f);
    EXPECT_FLOAT_EQ(output[4], 3.0f);
    EXPECT_FLOAT_EQ(output[6], 4.0f);
}

TEST_F(ResamplerTest, IntegerDownsample) {
    std::vector<float> input = {1.0f, 0.0f, 2.0f, 0.0f, 3.0f, 0.0f, 4.0f, 0.0f};
    auto output = Resampler::downsample(input, 2);

    EXPECT_EQ(output.size(), 4u);
    EXPECT_FLOAT_EQ(output[0], 1.0f);
    EXPECT_FLOAT_EQ(output[1], 2.0f);
    EXPECT_FLOAT_EQ(output[2], 3.0f);
    EXPECT_FLOAT_EQ(output[3], 4.0f);
}

TEST_F(ResamplerTest, UpsampleFactorOne) {
    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    auto output = Resampler::upsample(input, 1);
    EXPECT_EQ(output, input);
}

TEST_F(ResamplerTest, DownsampleFactorOne) {
    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    auto output = Resampler::downsample(input, 1);
    EXPECT_EQ(output, input);
}

TEST_F(ResamplerTest, StreamingReset) {
    resampler->setRatio(44100.0, 48000.0);
    resampler->reset();
    EXPECT_NEAR(resampler->getRatio(), 44100.0 / 48000.0, 0.001);
}

TEST_F(ResamplerTest, QualityLevels) {
    // Test all quality levels work
    std::vector<float> input(100, 0.5f);

    resampler->setQuality(ResamplerQuality::Fast);
    auto fast = resampler->resample(input, 44100.0, 48000.0);
    EXPECT_FALSE(fast.empty());

    resampler->setQuality(ResamplerQuality::Medium);
    auto medium = resampler->resample(input, 44100.0, 48000.0);
    EXPECT_FALSE(medium.empty());

    resampler->setQuality(ResamplerQuality::High);
    auto high = resampler->resample(input, 44100.0, 48000.0);
    EXPECT_FALSE(high.empty());

    resampler->setQuality(ResamplerQuality::Best);
    auto best = resampler->resample(input, 44100.0, 48000.0);
    EXPECT_FALSE(best.empty());
}

} // namespace test
} // namespace nap
