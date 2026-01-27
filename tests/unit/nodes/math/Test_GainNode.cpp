#include <gtest/gtest.h>
#include "../../../../src/nodes/math/GainNode.h"

namespace nap { namespace test {

TEST(GainNodeTest, DefaultGainIsUnity) {
    GainNode node;
    EXPECT_FLOAT_EQ(node.getGain(), 1.0f);
}

TEST(GainNodeTest, CanSetGain) {
    GainNode node;
    node.setGain(0.5f);
    EXPECT_FLOAT_EQ(node.getGain(), 0.5f);
}

TEST(GainNodeTest, CanSetGainInDb) {
    GainNode node;
    node.setGainDb(-6.0f);
    EXPECT_NEAR(node.getGain(), 0.5f, 0.01f);
}

TEST(GainNodeTest, HasCorrectTypeName) {
    GainNode node;
    EXPECT_EQ(node.getTypeName(), "GainNode");
}

TEST(GainNodeTest, ProcessAppliesGain) {
    GainNode node;
    node.setGain(0.5f);
    node.prepare(44100.0, 512);

    float input[] = {1.0f, 1.0f};
    float output[2];
    node.process(input, output, 1, 2);
    EXPECT_LT(output[0], 1.0f);
}

}} // namespace nap::test
