#include <gtest/gtest.h>
#include "../../../../src/nodes/source/ImpulseGenerator.h"

namespace nap { namespace test {

TEST(ImpulseGeneratorTest, DefaultAmplitudeIsOne) {
    ImpulseGenerator gen;
    EXPECT_FLOAT_EQ(gen.getAmplitude(), 1.0f);
}

TEST(ImpulseGeneratorTest, CanSetAmplitude) {
    ImpulseGenerator gen;
    gen.setAmplitude(0.5f);
    EXPECT_FLOAT_EQ(gen.getAmplitude(), 0.5f);
}

TEST(ImpulseGeneratorTest, DefaultOneShotIsFalse) {
    ImpulseGenerator gen;
    EXPECT_FALSE(gen.isOneShot());
}

TEST(ImpulseGeneratorTest, CanEnableOneShot) {
    ImpulseGenerator gen;
    gen.setOneShot(true);
    EXPECT_TRUE(gen.isOneShot());
}

TEST(ImpulseGeneratorTest, CanTrigger) {
    ImpulseGenerator gen;
    gen.trigger(); // Should not throw
}

TEST(ImpulseGeneratorTest, HasCorrectTypeName) {
    ImpulseGenerator gen;
    EXPECT_EQ(gen.getTypeName(), "ImpulseGenerator");
}

}} // namespace nap::test
