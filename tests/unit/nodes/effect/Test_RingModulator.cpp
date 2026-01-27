#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/RingModulator.h"

namespace nap { namespace test {

TEST(RingModulatorTest, DefaultFrequency) {
    RingModulator mod;
    EXPECT_FLOAT_EQ(mod.getFrequency(), 440.0f);
}

TEST(RingModulatorTest, CanSetFrequency) {
    RingModulator mod;
    mod.setFrequency(880.0f);
    EXPECT_FLOAT_EQ(mod.getFrequency(), 880.0f);
}

TEST(RingModulatorTest, DefaultWaveformIsSine) {
    RingModulator mod;
    EXPECT_EQ(mod.getWaveform(), RingModulator::WaveformType::Sine);
}

TEST(RingModulatorTest, CanSetWaveform) {
    RingModulator mod;
    mod.setWaveform(RingModulator::WaveformType::Square);
    EXPECT_EQ(mod.getWaveform(), RingModulator::WaveformType::Square);
}

TEST(RingModulatorTest, HasCorrectTypeName) {
    RingModulator mod;
    EXPECT_EQ(mod.getTypeName(), "RingModulator");
}

}} // namespace nap::test
