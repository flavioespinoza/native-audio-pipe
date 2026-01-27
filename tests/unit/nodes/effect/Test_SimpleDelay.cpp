#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/SimpleDelay.h"

namespace nap { namespace test {

TEST(SimpleDelayTest, DefaultDelayTime) {
    SimpleDelay delay;
    EXPECT_FLOAT_EQ(delay.getDelayTime(), 250.0f);
}

TEST(SimpleDelayTest, CanSetDelayTime) {
    SimpleDelay delay;
    delay.setDelayTime(500.0f);
    EXPECT_FLOAT_EQ(delay.getDelayTime(), 500.0f);
}

TEST(SimpleDelayTest, FeedbackIsClampedToValidRange) {
    SimpleDelay delay;
    delay.setFeedback(1.5f);
    EXPECT_LE(delay.getFeedback(), 0.99f);
}

TEST(SimpleDelayTest, MixIsClampedToValidRange) {
    SimpleDelay delay;
    delay.setMix(1.5f);
    EXPECT_LE(delay.getMix(), 1.0f);
}

TEST(SimpleDelayTest, HasCorrectTypeName) {
    SimpleDelay delay;
    EXPECT_EQ(delay.getTypeName(), "SimpleDelay");
}

}} // namespace nap::test
