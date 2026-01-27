#include <gtest/gtest.h>
#include "../../../../src/nodes/math/MixerNode.h"

namespace nap { namespace test {

TEST(MixerNodeTest, DefaultInputCountIsTwo) {
    MixerNode node;
    EXPECT_EQ(node.getNumInputs(), 2);
}

TEST(MixerNodeTest, CanSpecifyInputCount) {
    MixerNode node(4);
    EXPECT_EQ(node.getNumInputs(), 4);
}

TEST(MixerNodeTest, CanSetInputGain) {
    MixerNode node(2);
    node.setInputGain(0, 0.5f);
    EXPECT_FLOAT_EQ(node.getInputGain(0), 0.5f);
}

TEST(MixerNodeTest, CanMuteInput) {
    MixerNode node(2);
    node.muteInput(0, true);
    EXPECT_TRUE(node.isInputMuted(0));
}

TEST(MixerNodeTest, HasCorrectTypeName) {
    MixerNode node;
    EXPECT_EQ(node.getTypeName(), "MixerNode");
}

}} // namespace nap::test
