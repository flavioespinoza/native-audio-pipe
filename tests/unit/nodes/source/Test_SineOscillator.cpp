#include <gtest/gtest.h>
#include "../../../../src/nodes/source/SineOscillator.h"

namespace nap { namespace test {

TEST(SineOscillatorTest, DefaultFrequencyIs440) {
    SineOscillator osc;
    EXPECT_FLOAT_EQ(osc.getFrequency(), 440.0f);
}

TEST(SineOscillatorTest, CanSetFrequency) {
    SineOscillator osc;
    osc.setFrequency(880.0f);
    EXPECT_FLOAT_EQ(osc.getFrequency(), 880.0f);
}

TEST(SineOscillatorTest, DefaultAmplitudeIsOne) {
    SineOscillator osc;
    EXPECT_FLOAT_EQ(osc.getAmplitude(), 1.0f);
}

TEST(SineOscillatorTest, CanStartAndStop) {
    SineOscillator osc;
    EXPECT_FALSE(osc.isActive());
    osc.start();
    EXPECT_TRUE(osc.isActive());
    osc.stop();
    EXPECT_FALSE(osc.isActive());
}

TEST(SineOscillatorTest, HasCorrectTypeName) {
    SineOscillator osc;
    EXPECT_EQ(osc.getTypeName(), "SineOscillator");
}

}} // namespace nap::test
