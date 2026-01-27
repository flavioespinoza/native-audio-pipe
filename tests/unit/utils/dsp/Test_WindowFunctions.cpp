#include <gtest/gtest.h>
#include "utils/dsp/WindowFunctions.h"
#include <numeric>
#include <cmath>

namespace nap {
namespace test {

class WindowFunctionsTest : public ::testing::Test {
protected:
    const size_t windowSize = 1024;
};

TEST_F(WindowFunctionsTest, RectangularWindow) {
    auto window = WindowFunctions::rectangular(windowSize);

    EXPECT_EQ(window.size(), windowSize);
    for (float sample : window) {
        EXPECT_FLOAT_EQ(sample, 1.0f);
    }
}

TEST_F(WindowFunctionsTest, HannWindow) {
    auto window = WindowFunctions::hann(windowSize);

    EXPECT_EQ(window.size(), windowSize);

    // Endpoints should be zero (or very close)
    EXPECT_NEAR(window[0], 0.0f, 0.001f);
    EXPECT_NEAR(window[windowSize - 1], 0.0f, 0.001f);

    // Center should be 1.0
    EXPECT_NEAR(window[windowSize / 2], 1.0f, 0.001f);
}

TEST_F(WindowFunctionsTest, HammingWindow) {
    auto window = WindowFunctions::hamming(windowSize);

    EXPECT_EQ(window.size(), windowSize);

    // Hamming has non-zero endpoints (0.08)
    EXPECT_NEAR(window[0], 0.08f, 0.01f);
    EXPECT_NEAR(window[windowSize - 1], 0.08f, 0.01f);

    // Center should be 1.0
    EXPECT_NEAR(window[windowSize / 2], 1.0f, 0.001f);
}

TEST_F(WindowFunctionsTest, BlackmanWindow) {
    auto window = WindowFunctions::blackman(windowSize);

    EXPECT_EQ(window.size(), windowSize);

    // Endpoints should be near zero
    EXPECT_NEAR(window[0], 0.0f, 0.01f);

    // Center should be 1.0
    EXPECT_NEAR(window[windowSize / 2], 1.0f, 0.001f);
}

TEST_F(WindowFunctionsTest, BlackmanHarrisWindow) {
    auto window = WindowFunctions::blackmanHarris(windowSize);

    EXPECT_EQ(window.size(), windowSize);

    // Verify it's in valid range
    for (float sample : window) {
        EXPECT_GE(sample, 0.0f);
        EXPECT_LE(sample, 1.0f);
    }
}

TEST_F(WindowFunctionsTest, KaiserWindow) {
    auto window = WindowFunctions::kaiser(windowSize, 8.6f);

    EXPECT_EQ(window.size(), windowSize);

    // Endpoints should be small
    EXPECT_LT(window[0], 0.01f);

    // Center should be 1.0
    EXPECT_NEAR(window[windowSize / 2], 1.0f, 0.001f);
}

TEST_F(WindowFunctionsTest, TriangularWindow) {
    auto window = WindowFunctions::triangular(windowSize);

    EXPECT_EQ(window.size(), windowSize);

    // Should form a triangle
    // Endpoints should be small
    EXPECT_LT(window[0], 0.01f);

    // Center should be 1.0
    EXPECT_NEAR(window[windowSize / 2], 1.0f, 0.01f);
}

TEST_F(WindowFunctionsTest, GaussianWindow) {
    auto window = WindowFunctions::gaussian(windowSize, 0.4f);

    EXPECT_EQ(window.size(), windowSize);

    // Center should be 1.0
    EXPECT_NEAR(window[windowSize / 2], 1.0f, 0.001f);

    // Should be symmetric
    EXPECT_NEAR(window[100], window[windowSize - 1 - 100], 0.001f);
}

TEST_F(WindowFunctionsTest, TukeyWindow) {
    auto window = WindowFunctions::tukey(windowSize, 0.5f);

    EXPECT_EQ(window.size(), windowSize);

    // Center region should be 1.0
    EXPECT_NEAR(window[windowSize / 2], 1.0f, 0.001f);
}

TEST_F(WindowFunctionsTest, GenerateByType) {
    auto hann1 = WindowFunctions::generate(WindowType::Hann, windowSize);
    auto hann2 = WindowFunctions::hann(windowSize);

    EXPECT_EQ(hann1.size(), hann2.size());
    for (size_t i = 0; i < hann1.size(); ++i) {
        EXPECT_FLOAT_EQ(hann1[i], hann2[i]);
    }
}

TEST_F(WindowFunctionsTest, ApplyWindow) {
    std::vector<float> signal(windowSize, 1.0f);
    auto window = WindowFunctions::hann(windowSize);

    WindowFunctions::apply(signal.data(), window.data(), windowSize);

    // Signal should now equal the window
    for (size_t i = 0; i < windowSize; ++i) {
        EXPECT_FLOAT_EQ(signal[i], window[i]);
    }
}

TEST_F(WindowFunctionsTest, ApplyWindowByType) {
    std::vector<float> signal(windowSize, 1.0f);
    auto result = WindowFunctions::apply(signal, WindowType::Hamming);

    auto expected = WindowFunctions::hamming(windowSize);
    for (size_t i = 0; i < windowSize; ++i) {
        EXPECT_NEAR(result[i], expected[i], 0.001f);
    }
}

TEST_F(WindowFunctionsTest, WindowProperties) {
    // Coherent gain
    EXPECT_NEAR(WindowFunctions::getCoherentGain(WindowType::Hann), 0.5f, 0.01f);
    EXPECT_NEAR(WindowFunctions::getCoherentGain(WindowType::Rectangular), 1.0f, 0.01f);

    // ENBW
    EXPECT_GT(WindowFunctions::getEquivalentNoiseBandwidth(WindowType::Hann), 1.0f);

    // Recommended overlap
    EXPECT_NEAR(WindowFunctions::getRecommendedOverlap(WindowType::Hann), 0.5f, 0.01f);
}

TEST_F(WindowFunctionsTest, COLAWindow) {
    auto window = WindowFunctions::generateCOLA(WindowType::Hann, 1024, 0.5f);
    EXPECT_EQ(window.size(), 1024u);
}

TEST_F(WindowFunctionsTest, WindowSymmetry) {
    auto window = WindowFunctions::hann(1024);

    // Hann window should be symmetric
    for (size_t i = 0; i < 512; ++i) {
        EXPECT_NEAR(window[i], window[1023 - i], 0.0001f);
    }
}

} // namespace test
} // namespace nap
