#include <gtest/gtest.h>
#include "../../../../src/core/graph/FeedbackLoopDetector.h"
#include "../../../../src/core/graph/ConnectionManager.h"

namespace nap {
namespace test {

class FeedbackLoopDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector = std::make_unique<FeedbackLoopDetector>();
        connectionManager = std::make_unique<ConnectionManager>();
    }

    std::unique_ptr<FeedbackLoopDetector> detector;
    std::unique_ptr<ConnectionManager> connectionManager;
};

TEST_F(FeedbackLoopDetectorTest, EmptyGraphHasNoCycle) {
    std::vector<std::string> nodes;
    EXPECT_FALSE(detector->hasCycle(nodes, *connectionManager));
}

TEST_F(FeedbackLoopDetectorTest, LinearChainHasNoCycle) {
    std::vector<std::string> nodes = {"A", "B", "C"};
    connectionManager->addConnection("A", 0, "B", 0);
    connectionManager->addConnection("B", 0, "C", 0);
    EXPECT_FALSE(detector->hasCycle(nodes, *connectionManager));
}

TEST_F(FeedbackLoopDetectorTest, DetectsCycle) {
    std::vector<std::string> nodes = {"A", "B", "C"};
    connectionManager->addConnection("A", 0, "B", 0);
    connectionManager->addConnection("B", 0, "C", 0);
    connectionManager->addConnection("C", 0, "A", 0);
    EXPECT_TRUE(detector->hasCycle(nodes, *connectionManager));
}

TEST_F(FeedbackLoopDetectorTest, WouldCreateCycleCheck) {
    std::vector<std::string> nodes = {"A", "B", "C"};
    connectionManager->addConnection("A", 0, "B", 0);
    connectionManager->addConnection("B", 0, "C", 0);
    EXPECT_TRUE(detector->wouldCreateCycle("C", "A", nodes, *connectionManager));
    EXPECT_FALSE(detector->wouldCreateCycle("A", "C", nodes, *connectionManager));
}

} // namespace test
} // namespace nap
