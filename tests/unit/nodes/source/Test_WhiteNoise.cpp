#include <gtest/gtest.h>
#include "../../../../src/nodes/source/WhiteNoise.h"

namespace nap { namespace test {

TEST(WhiteNoiseTest, DefaultAmplitudeIsOne) {
    WhiteNoise noise;
    EXPECT_FLOAT_EQ(noise.getAmplitude(), 1.0f);
}

TEST(WhiteNoiseTest, CanSetAmplitude) {
    WhiteNoise noise;
    noise.setAmplitude(0.5f);
    EXPECT_FLOAT_EQ(noise.getAmplitude(), 0.5f);
}

TEST(WhiteNoiseTest, CanStartAndStop) {
    WhiteNoise noise;
    EXPECT_FALSE(noise.isActive());
    noise.start();
    EXPECT_TRUE(noise.isActive());
    noise.stop();
    EXPECT_FALSE(noise.isActive());
}

TEST(WhiteNoiseTest, GeneratesOutput) {
    WhiteNoise noise;
    noise.prepare(44100.0, 512);
    noise.start();

    float output[2];
    noise.generate(output, 1, 2);
    // Just verify it generates something
}

TEST(WhiteNoiseTest, HasCorrectTypeName) {
    WhiteNoise noise;
    EXPECT_EQ(noise.getTypeName(), "WhiteNoise");
}

}} // namespace nap::test
