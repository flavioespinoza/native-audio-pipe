#include <gtest/gtest.h>
#include "../../../../src/nodes/math/SplitterNode.h"

namespace nap { namespace test {

TEST(SplitterNodeTest, DefaultOutputCountIsTwo) {
    SplitterNode node;
    EXPECT_EQ(node.getNumOutputs(), 2);
}

TEST(SplitterNodeTest, CanSpecifyOutputCount) {
    SplitterNode node(4);
    EXPECT_EQ(node.getNumOutputs(), 4);
}

TEST(SplitterNodeTest, CanSetOutputGain) {
    SplitterNode node(2);
    node.setOutputGain(0, 0.5f);
    EXPECT_FLOAT_EQ(node.getOutputGain(0), 0.5f);
}

TEST(SplitterNodeTest, CanMuteOutput) {
    SplitterNode node(2);
    node.muteOutput(0, true);
    EXPECT_TRUE(node.isOutputMuted(0));
}

TEST(SplitterNodeTest, HasCorrectTypeName) {
    SplitterNode node;
    EXPECT_EQ(node.getTypeName(), "SplitterNode");
}

}} // namespace nap::test
