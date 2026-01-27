#include <gtest/gtest.h>
#include "../../../../src/nodes/math/InverterNode.h"

namespace nap { namespace test {

TEST(InverterNodeTest, DefaultInvertsBothChannels) {
    InverterNode node;
    EXPECT_TRUE(node.getInvertLeft());
    EXPECT_TRUE(node.getInvertRight());
}

TEST(InverterNodeTest, CanSetLeftInvert) {
    InverterNode node;
    node.setInvertLeft(false);
    EXPECT_FALSE(node.getInvertLeft());
}

TEST(InverterNodeTest, CanSetRightInvert) {
    InverterNode node;
    node.setInvertRight(false);
    EXPECT_FALSE(node.getInvertRight());
}

TEST(InverterNodeTest, ProcessInvertsSignal) {
    InverterNode node;
    node.prepare(44100.0, 512);

    float input[] = {1.0f, 1.0f};
    float output[2];
    node.process(input, output, 1, 2);
    EXPECT_FLOAT_EQ(output[0], -1.0f);
    EXPECT_FLOAT_EQ(output[1], -1.0f);
}

TEST(InverterNodeTest, HasCorrectTypeName) {
    InverterNode node;
    EXPECT_EQ(node.getTypeName(), "InverterNode");
}

}} // namespace nap::test
