#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/Flanger.h"

namespace nap { namespace test {

TEST(FlangerTest, DefaultRate) {
    Flanger flanger;
    EXPECT_FLOAT_EQ(flanger.getRate(), 0.25f);
}

TEST(FlangerTest, CanSetRate) {
    Flanger flanger;
    flanger.setRate(1.0f);
    EXPECT_FLOAT_EQ(flanger.getRate(), 1.0f);
}

TEST(FlangerTest, DefaultDelay) {
    Flanger flanger;
    EXPECT_FLOAT_EQ(flanger.getDelay(), 5.0f);
}

TEST(FlangerTest, CanSetDelay) {
    Flanger flanger;
    flanger.setDelay(10.0f);
    EXPECT_FLOAT_EQ(flanger.getDelay(), 10.0f);
}

TEST(FlangerTest, HasCorrectTypeName) {
    Flanger flanger;
    EXPECT_EQ(flanger.getTypeName(), "Flanger");
}

}} // namespace nap::test
