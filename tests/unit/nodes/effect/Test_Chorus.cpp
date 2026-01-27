#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/Chorus.h"

namespace nap { namespace test {

TEST(ChorusTest, DefaultVoices) {
    Chorus chorus;
    EXPECT_EQ(chorus.getVoices(), 3);
}

TEST(ChorusTest, CanSetVoices) {
    Chorus chorus;
    chorus.setVoices(5);
    EXPECT_EQ(chorus.getVoices(), 5);
}

TEST(ChorusTest, DefaultRate) {
    Chorus chorus;
    EXPECT_FLOAT_EQ(chorus.getRate(), 1.5f);
}

TEST(ChorusTest, CanSetRate) {
    Chorus chorus;
    chorus.setRate(2.0f);
    EXPECT_FLOAT_EQ(chorus.getRate(), 2.0f);
}

TEST(ChorusTest, HasCorrectTypeName) {
    Chorus chorus;
    EXPECT_EQ(chorus.getTypeName(), "Chorus");
}

}} // namespace nap::test
