#include <gtest/gtest.h>
#include "../../../../src/core/graph/ConnectionManager.h"

namespace nap {
namespace test {

class ConnectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<ConnectionManager>();
    }

    std::unique_ptr<ConnectionManager> manager;
};

TEST_F(ConnectionManagerTest, InitialStateIsEmpty) {
    EXPECT_EQ(manager->getConnectionCount(), 0);
}

TEST_F(ConnectionManagerTest, CanAddConnection) {
    EXPECT_TRUE(manager->addConnection("node1", 0, "node2", 0));
    EXPECT_EQ(manager->getConnectionCount(), 1);
}

TEST_F(ConnectionManagerTest, CanRemoveConnection) {
    manager->addConnection("node1", 0, "node2", 0);
    EXPECT_TRUE(manager->removeConnection("node1", 0, "node2", 0));
    EXPECT_EQ(manager->getConnectionCount(), 0);
}

TEST_F(ConnectionManagerTest, CanCheckConnectionExists) {
    manager->addConnection("node1", 0, "node2", 0);
    EXPECT_TRUE(manager->hasConnection("node1", 0, "node2", 0));
    EXPECT_FALSE(manager->hasConnection("node1", 0, "node3", 0));
}

TEST_F(ConnectionManagerTest, CanGetConnectionsFrom) {
    manager->addConnection("node1", 0, "node2", 0);
    manager->addConnection("node1", 1, "node3", 0);
    auto connections = manager->getConnectionsFrom("node1");
    EXPECT_EQ(connections.size(), 2);
}

} // namespace test
} // namespace nap
