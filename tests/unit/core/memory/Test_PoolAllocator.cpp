#include <gtest/gtest.h>
#include "../../../../src/core/memory/PoolAllocator.h"

namespace nap {
namespace test {

class PoolAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        allocator = std::make_unique<PoolAllocator>(64, 32, 16);
    }

    std::unique_ptr<PoolAllocator> allocator;
};

TEST_F(PoolAllocatorTest, HasCorrectConfiguration) {
    EXPECT_EQ(allocator->getBlockSize(), 64);
    EXPECT_EQ(allocator->getTotalBlocks(), 32);
    EXPECT_EQ(allocator->getAlignment(), 16);
}

TEST_F(PoolAllocatorTest, CanAllocate) {
    void* ptr = allocator->allocate();
    EXPECT_NE(ptr, nullptr);
}

TEST_F(PoolAllocatorTest, CanDeallocate) {
    void* ptr = allocator->allocate();
    size_t before = allocator->getAvailableBlocks();
    allocator->deallocate(ptr);
    EXPECT_EQ(allocator->getAvailableBlocks(), before + 1);
}

TEST_F(PoolAllocatorTest, OwnsAllocatedPointer) {
    void* ptr = allocator->allocate();
    EXPECT_TRUE(allocator->owns(ptr));
}

TEST_F(PoolAllocatorTest, DoesNotOwnExternalPointer) {
    int x;
    EXPECT_FALSE(allocator->owns(&x));
}

} // namespace test
} // namespace nap
