#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/BitCrusher.h"

namespace nap { namespace test {

TEST(BitCrusherTest, DefaultBitDepth) {
    BitCrusher crusher;
    EXPECT_EQ(crusher.getBitDepth(), 8);
}

TEST(BitCrusherTest, CanSetBitDepth) {
    BitCrusher crusher;
    crusher.setBitDepth(4);
    EXPECT_EQ(crusher.getBitDepth(), 4);
}

TEST(BitCrusherTest, BitDepthIsClampedToValidRange) {
    BitCrusher crusher;
    crusher.setBitDepth(32);
    EXPECT_LE(crusher.getBitDepth(), 16);
}

TEST(BitCrusherTest, DefaultDownsampleFactor) {
    BitCrusher crusher;
    EXPECT_EQ(crusher.getDownsampleFactor(), 1);
}

TEST(BitCrusherTest, HasCorrectTypeName) {
    BitCrusher crusher;
    EXPECT_EQ(crusher.getTypeName(), "BitCrusher");
}

}} // namespace nap::test
