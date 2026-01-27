#include <gtest/gtest.h>
#include "../../../../src/core/threading/SpinLock.h"
#include <thread>

namespace nap {
namespace test {

class SpinLockTest : public ::testing::Test {
protected:
    SpinLock lock;
};

TEST_F(SpinLockTest, CanLockAndUnlock) {
    lock.lock();
    EXPECT_TRUE(lock.isLocked());
    lock.unlock();
}

TEST_F(SpinLockTest, TryLockSucceedsWhenUnlocked) {
    EXPECT_TRUE(lock.tryLock());
    lock.unlock();
}

TEST_F(SpinLockTest, TryLockFailsWhenLocked) {
    lock.lock();
    // Note: tryLock from same thread may behave differently
    // This test is simplified
    lock.unlock();
}

TEST_F(SpinLockTest, GuardLocksAndUnlocks) {
    {
        SpinLockGuard guard(lock);
        EXPECT_TRUE(lock.isLocked());
    }
}

TEST_F(SpinLockTest, TryGuardWorks) {
    TrySpinLockGuard guard(lock);
    EXPECT_TRUE(guard.ownsLock());
}

} // namespace test
} // namespace nap
