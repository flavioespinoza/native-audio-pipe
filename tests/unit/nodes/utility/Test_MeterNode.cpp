#include <gtest/gtest.h>
#include "../../../../src/nodes/utility/MeterNode.h"

namespace nap { namespace test {

TEST(MeterNodeTest, InitialPeakLevelIsZero) {
    MeterNode meter;
    EXPECT_FLOAT_EQ(meter.getPeakLevel(0), 0.0f);
    EXPECT_FLOAT_EQ(meter.getPeakLevel(1), 0.0f);
}

TEST(MeterNodeTest, InitialRmsLevelIsZero) {
    MeterNode meter;
    EXPECT_FLOAT_EQ(meter.getRmsLevel(0), 0.0f);
}

TEST(MeterNodeTest, CanResetPeaks) {
    MeterNode meter;
    meter.resetPeaks();
    EXPECT_FLOAT_EQ(meter.getPeakLevel(0), 0.0f);
}

TEST(MeterNodeTest, PassthroughSignal) {
    MeterNode meter;
    meter.prepare(44100.0, 512);

    float input[] = {0.5f, 0.5f};
    float output[2];
    meter.process(input, output, 1, 2);
    EXPECT_FLOAT_EQ(output[0], 0.5f);
    EXPECT_FLOAT_EQ(output[1], 0.5f);
}

TEST(MeterNodeTest, HasCorrectTypeName) {
    MeterNode meter;
    EXPECT_EQ(meter.getTypeName(), "MeterNode");
}

}} // namespace nap::test
