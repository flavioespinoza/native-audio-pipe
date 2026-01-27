#include <gtest/gtest.h>
#include "../../../../src/core/threading/WorkerThread.h"
#include <atomic>

namespace nap {
namespace test {

class WorkerThreadTest : public ::testing::Test {
protected:
    void SetUp() override {
        worker = std::make_unique<WorkerThread>("TestWorker");
    }

    void TearDown() override {
        if (worker->isRunning()) {
            worker->stop(true);
        }
    }

    std::unique_ptr<WorkerThread> worker;
};

TEST_F(WorkerThreadTest, InitialStateIsNotRunning) {
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(WorkerThreadTest, CanStart) {
    EXPECT_TRUE(worker->start());
    EXPECT_TRUE(worker->isRunning());
}

TEST_F(WorkerThreadTest, CanStop) {
    worker->start();
    worker->stop(true);
    EXPECT_FALSE(worker->isRunning());
}

TEST_F(WorkerThreadTest, ExecutesTask) {
    std::atomic<int> counter{0};
    worker->setTask([&counter]() { counter++; });
    worker->start();
    worker->wake();
    worker->waitForCompletion(1000);
    EXPECT_GE(counter.load(), 1);
}

TEST_F(WorkerThreadTest, HasCorrectName) {
    EXPECT_EQ(worker->getName(), "TestWorker");
}

} // namespace test
} // namespace nap
