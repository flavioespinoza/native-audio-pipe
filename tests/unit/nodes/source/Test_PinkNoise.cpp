#include <gtest/gtest.h>
#include "../../../../src/nodes/source/PinkNoise.h"

namespace nap { namespace test {

TEST(PinkNoiseTest, DefaultAmplitudeIsOne) {
    PinkNoise noise;
    EXPECT_FLOAT_EQ(noise.getAmplitude(), 1.0f);
}

TEST(PinkNoiseTest, CanSetAmplitude) {
    PinkNoise noise;
    noise.setAmplitude(0.5f);
    EXPECT_FLOAT_EQ(noise.getAmplitude(), 0.5f);
}

TEST(PinkNoiseTest, CanStartAndStop) {
    PinkNoise noise;
    EXPECT_FALSE(noise.isActive());
    noise.start();
    EXPECT_TRUE(noise.isActive());
}

TEST(PinkNoiseTest, HasCorrectTypeName) {
    PinkNoise noise;
    EXPECT_EQ(noise.getTypeName(), "PinkNoise");
}

}} // namespace nap::test
