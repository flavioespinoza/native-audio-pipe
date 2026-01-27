#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/HardClipper.h"

namespace nap { namespace test {

TEST(HardClipperTest, DefaultThreshold) {
    HardClipper clipper;
    EXPECT_FLOAT_EQ(clipper.getThreshold(), 0.8f);
}

TEST(HardClipperTest, CanSetThreshold) {
    HardClipper clipper;
    clipper.setThreshold(0.5f);
    EXPECT_FLOAT_EQ(clipper.getThreshold(), 0.5f);
}

TEST(HardClipperTest, ThresholdIsClampedToValidRange) {
    HardClipper clipper;
    clipper.setThreshold(1.5f);
    EXPECT_LE(clipper.getThreshold(), 1.0f);
}

TEST(HardClipperTest, HasCorrectTypeName) {
    HardClipper clipper;
    EXPECT_EQ(clipper.getTypeName(), "HardClipper");
}

}} // namespace nap::test
