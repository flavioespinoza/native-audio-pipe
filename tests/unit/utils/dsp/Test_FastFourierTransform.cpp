#include <gtest/gtest.h>
#include "utils/dsp/FastFourierTransform.h"
#include <cmath>

namespace nap {
namespace test {

class FastFourierTransformTest : public ::testing::Test {
protected:
    void SetUp() override {
        fft = std::make_unique<FastFourierTransform>(1024);
    }

    std::unique_ptr<FastFourierTransform> fft;
};

TEST_F(FastFourierTransformTest, Construction) {
    EXPECT_EQ(fft->getSize(), 1024u);
}

TEST_F(FastFourierTransformTest, SetSize) {
    fft->setSize(2048);
    EXPECT_EQ(fft->getSize(), 2048u);
}

TEST_F(FastFourierTransformTest, NonPowerOfTwoRoundsUp) {
    FastFourierTransform fft2(1000);
    EXPECT_EQ(fft2.getSize(), 1024u);  // Rounded to next power of two
}

TEST_F(FastFourierTransformTest, IsPowerOfTwo) {
    EXPECT_TRUE(FastFourierTransform::isPowerOfTwo(1));
    EXPECT_TRUE(FastFourierTransform::isPowerOfTwo(2));
    EXPECT_TRUE(FastFourierTransform::isPowerOfTwo(256));
    EXPECT_TRUE(FastFourierTransform::isPowerOfTwo(1024));

    EXPECT_FALSE(FastFourierTransform::isPowerOfTwo(0));
    EXPECT_FALSE(FastFourierTransform::isPowerOfTwo(3));
    EXPECT_FALSE(FastFourierTransform::isPowerOfTwo(1000));
}

TEST_F(FastFourierTransformTest, NextPowerOfTwo) {
    EXPECT_EQ(FastFourierTransform::nextPowerOfTwo(1), 1u);
    EXPECT_EQ(FastFourierTransform::nextPowerOfTwo(2), 2u);
    EXPECT_EQ(FastFourierTransform::nextPowerOfTwo(3), 4u);
    EXPECT_EQ(FastFourierTransform::nextPowerOfTwo(1000), 1024u);
    EXPECT_EQ(FastFourierTransform::nextPowerOfTwo(1024), 1024u);
}

TEST_F(FastFourierTransformTest, ForwardTransformDC) {
    // DC signal should produce energy only in bin 0
    std::vector<float> dcSignal(1024, 1.0f);
    auto spectrum = fft->forward(dcSignal);

    EXPECT_NEAR(std::abs(spectrum[0]), 1024.0f, 1.0f);

    // Other bins should be near zero
    for (size_t i = 1; i < spectrum.size() / 2; ++i) {
        EXPECT_NEAR(std::abs(spectrum[i]), 0.0f, 0.01f);
    }
}

TEST_F(FastFourierTransformTest, ForwardTransformSine) {
    // Pure sine wave should produce energy at specific frequency bin
    const size_t size = 1024;
    const float frequency = 10.0f;  // 10 cycles in buffer
    std::vector<float> sineSignal(size);

    for (size_t i = 0; i < size; ++i) {
        sineSignal[i] = std::sin(2.0f * static_cast<float>(M_PI) * frequency * i / size);
    }

    auto spectrum = fft->forward(sineSignal);

    // Energy should be concentrated around bin 10
    float peakMagnitude = 0.0f;
    size_t peakBin = 0;
    for (size_t i = 0; i < size / 2; ++i) {
        float mag = std::abs(spectrum[i]);
        if (mag > peakMagnitude) {
            peakMagnitude = mag;
            peakBin = i;
        }
    }

    EXPECT_EQ(peakBin, 10u);
}

TEST_F(FastFourierTransformTest, RoundTrip) {
    // Forward then inverse should recover original signal
    const size_t size = 256;
    std::vector<float> original(size);

    for (size_t i = 0; i < size; ++i) {
        original[i] = std::sin(2.0f * static_cast<float>(M_PI) * i / size) +
                      0.5f * std::cos(4.0f * static_cast<float>(M_PI) * i / size);
    }

    FastFourierTransform smallFft(size);
    auto spectrum = smallFft.forward(original);
    auto recovered = smallFft.inverse(spectrum);

    for (size_t i = 0; i < size; ++i) {
        EXPECT_NEAR(recovered[i], original[i], 0.001f);
    }
}

TEST_F(FastFourierTransformTest, GetFrequencyBin) {
    // At 44100 Hz sample rate with 1024-point FFT
    // 440 Hz should be at bin 440 * 1024 / 44100 ≈ 10
    size_t bin = FastFourierTransform::getFrequencyBin(440.0f, 44100.0f, 1024);
    EXPECT_EQ(bin, 10u);
}

TEST_F(FastFourierTransformTest, GetBinFrequency) {
    // Bin 10 at 44100 Hz sample rate with 1024-point FFT
    float freq = FastFourierTransform::getBinFrequency(10, 44100.0f, 1024);
    EXPECT_NEAR(freq, 430.66f, 1.0f);  // 10 * 44100 / 1024
}

TEST_F(FastFourierTransformTest, MagnitudesAndPhases) {
    const size_t size = 256;
    std::vector<float> signal(size);
    std::vector<float> magnitudes(size);
    std::vector<float> phases(size);

    for (size_t i = 0; i < size; ++i) {
        signal[i] = std::cos(2.0f * static_cast<float>(M_PI) * 8 * i / size);
    }

    FastFourierTransform smallFft(size);
    smallFft.forward(signal.data(), magnitudes.data(), phases.data());

    // Peak should be at bin 8
    float maxMag = 0.0f;
    size_t maxBin = 0;
    for (size_t i = 0; i < size / 2; ++i) {
        if (magnitudes[i] > maxMag) {
            maxMag = magnitudes[i];
            maxBin = i;
        }
    }
    EXPECT_EQ(maxBin, 8u);
}

} // namespace test
} // namespace nap
