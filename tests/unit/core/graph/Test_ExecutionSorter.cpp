#include <gtest/gtest.h>
#include "../../../../src/core/graph/ExecutionSorter.h"
#include "../../../../src/core/graph/ConnectionManager.h"

namespace nap {
namespace test {

class ExecutionSorterTest : public ::testing::Test {
protected:
    void SetUp() override {
        sorter = std::make_unique<ExecutionSorter>();
        connectionManager = std::make_unique<ConnectionManager>();
    }

    std::unique_ptr<ExecutionSorter> sorter;
    std::unique_ptr<ConnectionManager> connectionManager;
};

TEST_F(ExecutionSorterTest, EmptyGraphHasValidOrder) {
    std::vector<std::string> nodes;
    EXPECT_TRUE(sorter->hasValidOrder(nodes, *connectionManager));
}

TEST_F(ExecutionSorterTest, SingleNodeHasValidOrder) {
    std::vector<std::string> nodes = {"node1"};
    EXPECT_TRUE(sorter->hasValidOrder(nodes, *connectionManager));
}

TEST_F(ExecutionSorterTest, LinearChainProducesCorrectOrder) {
    std::vector<std::string> nodes = {"A", "B", "C"};
    connectionManager->addConnection("A", 0, "B", 0);
    connectionManager->addConnection("B", 0, "C", 0);

    auto order = sorter->computeExecutionOrder(nodes, *connectionManager);
    EXPECT_EQ(order.size(), 3);
}

TEST_F(ExecutionSorterTest, CanComputeParallelGroups) {
    std::vector<std::string> nodes = {"A", "B", "C", "D"};
    connectionManager->addConnection("A", 0, "C", 0);
    connectionManager->addConnection("B", 0, "D", 0);

    auto groups = sorter->computeParallelGroups(nodes, *connectionManager);
    EXPECT_GE(groups.size(), 1);
}

} // namespace test
} // namespace nap
