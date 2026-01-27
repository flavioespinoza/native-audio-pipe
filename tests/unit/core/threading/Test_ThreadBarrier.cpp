#include <gtest/gtest.h>
#include "../../../../src/core/threading/ThreadBarrier.h"
#include <atomic>
#include <thread>

namespace nap {
namespace test {

class ThreadBarrierTest : public ::testing::Test {
protected:
    void SetUp() override {
        barrier = std::make_unique<ThreadBarrier>(2);
    }

    std::unique_ptr<ThreadBarrier> barrier;
};

TEST_F(ThreadBarrierTest, HasCorrectThreadCount) {
    EXPECT_EQ(barrier->getNumThreads(), 2);
}

TEST_F(ThreadBarrierTest, CanReset) {
    barrier->reset();
    EXPECT_EQ(barrier->getWaitingCount(), 0);
}

TEST_F(ThreadBarrierTest, CanResetWithNewCount) {
    barrier->reset(4);
    EXPECT_EQ(barrier->getNumThreads(), 4);
}

TEST_F(ThreadBarrierTest, SynchronizesTwoThreads) {
    std::atomic<int> counter{0};

    std::thread t1([this, &counter]() {
        counter++;
        barrier->wait();
        EXPECT_EQ(counter.load(), 2);
    });

    std::thread t2([this, &counter]() {
        counter++;
        barrier->wait();
        EXPECT_EQ(counter.load(), 2);
    });

    t1.join();
    t2.join();
}

TEST_F(ThreadBarrierTest, CompletionFunctionIsCalled) {
    bool called = false;
    barrier->setCompletionFunction([&called]() { called = true; });

    std::thread t1([this]() { barrier->wait(); });
    std::thread t2([this]() { barrier->wait(); });

    t1.join();
    t2.join();

    EXPECT_TRUE(called);
}

} // namespace test
} // namespace nap
