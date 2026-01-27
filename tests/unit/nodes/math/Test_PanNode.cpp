#include <gtest/gtest.h>
#include "../../../../src/nodes/math/PanNode.h"

namespace nap { namespace test {

TEST(PanNodeTest, DefaultPanIsCenter) {
    PanNode node;
    EXPECT_FLOAT_EQ(node.getPan(), 0.0f);
}

TEST(PanNodeTest, CanSetPan) {
    PanNode node;
    node.setPan(-0.5f);
    EXPECT_FLOAT_EQ(node.getPan(), -0.5f);
}

TEST(PanNodeTest, PanIsClampedToValidRange) {
    PanNode node;
    node.setPan(2.0f);
    EXPECT_LE(node.getPan(), 1.0f);
    node.setPan(-2.0f);
    EXPECT_GE(node.getPan(), -1.0f);
}

TEST(PanNodeTest, HasCorrectTypeName) {
    PanNode node;
    EXPECT_EQ(node.getTypeName(), "PanNode");
}

TEST(PanNodeTest, CanSetPanLaw) {
    PanNode node;
    node.setPanLaw(PanNode::PanLaw::Linear);
    EXPECT_EQ(node.getPanLaw(), PanNode::PanLaw::Linear);
}

}} // namespace nap::test
