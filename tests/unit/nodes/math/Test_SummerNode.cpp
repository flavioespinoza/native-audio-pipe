#include <gtest/gtest.h>
#include "../../../../src/nodes/math/SummerNode.h"

namespace nap { namespace test {

TEST(SummerNodeTest, DefaultInputCountIsTwo) {
    SummerNode node;
    EXPECT_EQ(node.getNumInputs(), 2);
}

TEST(SummerNodeTest, CanSpecifyInputCount) {
    SummerNode node(4);
    EXPECT_EQ(node.getNumInputs(), 4);
}

TEST(SummerNodeTest, DefaultNormalizeIsFalse) {
    SummerNode node;
    EXPECT_FALSE(node.getNormalize());
}

TEST(SummerNodeTest, CanEnableNormalize) {
    SummerNode node;
    node.setNormalize(true);
    EXPECT_TRUE(node.getNormalize());
}

TEST(SummerNodeTest, DefaultClipIsTrue) {
    SummerNode node;
    EXPECT_TRUE(node.getClip());
}

TEST(SummerNodeTest, HasCorrectTypeName) {
    SummerNode node;
    EXPECT_EQ(node.getTypeName(), "SummerNode");
}

}} // namespace nap::test
