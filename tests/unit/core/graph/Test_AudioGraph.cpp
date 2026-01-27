#include <gtest/gtest.h>
#include "../../../../src/core/graph/AudioGraph.h"
#include "../../../../src/nodes/math/GainNode.h"

namespace nap {
namespace test {

class AudioGraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph = std::make_unique<AudioGraph>();
    }

    std::unique_ptr<AudioGraph> graph;
};

TEST_F(AudioGraphTest, InitialStateIsEmpty) {
    EXPECT_EQ(graph->getNodeCount(), 0);
}

TEST_F(AudioGraphTest, CanAddNode) {
    auto node = std::make_shared<GainNode>();
    EXPECT_TRUE(graph->addNode(node));
    EXPECT_EQ(graph->getNodeCount(), 1);
}

TEST_F(AudioGraphTest, CanRemoveNode) {
    auto node = std::make_shared<GainNode>();
    graph->addNode(node);
    EXPECT_TRUE(graph->removeNode(node->getNodeId()));
    EXPECT_EQ(graph->getNodeCount(), 0);
}

TEST_F(AudioGraphTest, CanRetrieveNode) {
    auto node = std::make_shared<GainNode>();
    graph->addNode(node);
    auto retrieved = graph->getNode(node->getNodeId());
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getNodeId(), node->getNodeId());
}

TEST_F(AudioGraphTest, CanConnectNodes) {
    auto gain1 = std::make_shared<GainNode>();
    auto gain2 = std::make_shared<GainNode>();
    graph->addNode(gain1);
    graph->addNode(gain2);
    EXPECT_TRUE(graph->connect(gain1->getNodeId(), 0, gain2->getNodeId(), 0));
}

} // namespace test
} // namespace nap
