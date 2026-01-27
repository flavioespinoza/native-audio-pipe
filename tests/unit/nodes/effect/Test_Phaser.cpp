#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/Phaser.h"

namespace nap { namespace test {

TEST(PhaserTest, DefaultRate) {
    Phaser phaser;
    EXPECT_FLOAT_EQ(phaser.getRate(), 0.5f);
}

TEST(PhaserTest, CanSetRate) {
    Phaser phaser;
    phaser.setRate(2.0f);
    EXPECT_FLOAT_EQ(phaser.getRate(), 2.0f);
}

TEST(PhaserTest, DefaultStages) {
    Phaser phaser;
    EXPECT_EQ(phaser.getStages(), 4);
}

TEST(PhaserTest, CanSetStages) {
    Phaser phaser;
    phaser.setStages(8);
    EXPECT_EQ(phaser.getStages(), 8);
}

TEST(PhaserTest, HasCorrectTypeName) {
    Phaser phaser;
    EXPECT_EQ(phaser.getTypeName(), "Phaser");
}

}} // namespace nap::test
