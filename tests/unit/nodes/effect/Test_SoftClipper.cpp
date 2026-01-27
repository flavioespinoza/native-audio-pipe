#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/SoftClipper.h"

namespace nap { namespace test {

TEST(SoftClipperTest, DefaultDrive) {
    SoftClipper clipper;
    EXPECT_FLOAT_EQ(clipper.getDrive(), 1.0f);
}

TEST(SoftClipperTest, CanSetDrive) {
    SoftClipper clipper;
    clipper.setDrive(2.0f);
    EXPECT_FLOAT_EQ(clipper.getDrive(), 2.0f);
}

TEST(SoftClipperTest, DefaultCurveTypeIsTanh) {
    SoftClipper clipper;
    EXPECT_EQ(clipper.getCurveType(), SoftClipper::CurveType::Tanh);
}

TEST(SoftClipperTest, CanSetCurveType) {
    SoftClipper clipper;
    clipper.setCurveType(SoftClipper::CurveType::Atan);
    EXPECT_EQ(clipper.getCurveType(), SoftClipper::CurveType::Atan);
}

TEST(SoftClipperTest, HasCorrectTypeName) {
    SoftClipper clipper;
    EXPECT_EQ(clipper.getTypeName(), "SoftClipper");
}

}} // namespace nap::test
