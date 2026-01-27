#include <gtest/gtest.h>
#include "../../../../src/core/threading/TaskQueue.h"

namespace nap {
namespace test {

class TaskQueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        queue = std::make_unique<TaskQueue>(64);
    }

    std::unique_ptr<TaskQueue> queue;
};

TEST_F(TaskQueueTest, InitialStateIsEmpty) {
    EXPECT_TRUE(queue->isEmpty());
    EXPECT_EQ(queue->size(), 0);
}

TEST_F(TaskQueueTest, CanPushTask) {
    EXPECT_TRUE(queue->push([]() {}));
    EXPECT_EQ(queue->size(), 1);
}

TEST_F(TaskQueueTest, CanPopTask) {
    int value = 0;
    queue->push([&value]() { value = 42; });

    TaskQueue::Task task;
    EXPECT_TRUE(queue->tryPop(task));
    task();
    EXPECT_EQ(value, 42);
}

TEST_F(TaskQueueTest, ExecuteAllRunsAllTasks) {
    int counter = 0;
    queue->push([&counter]() { counter++; });
    queue->push([&counter]() { counter++; });
    queue->push([&counter]() { counter++; });

    size_t executed = queue->executeAll();
    EXPECT_EQ(executed, 3);
    EXPECT_EQ(counter, 3);
}

TEST_F(TaskQueueTest, ClearRemovesAllTasks) {
    queue->push([]() {});
    queue->push([]() {});
    queue->clear();
    EXPECT_TRUE(queue->isEmpty());
}

} // namespace test
} // namespace nap
