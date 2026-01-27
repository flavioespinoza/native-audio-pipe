#include <gtest/gtest.h>
#include "../../../../src/core/memory/AudioBlockAllocator.h"

namespace nap {
namespace test {

class AudioBlockAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        allocator = std::make_unique<AudioBlockAllocator>(512, 2, 16);
    }

    std::unique_ptr<AudioBlockAllocator> allocator;
};

TEST_F(AudioBlockAllocatorTest, HasCorrectConfiguration) {
    EXPECT_EQ(allocator->getBlockSize(), 512);
    EXPECT_EQ(allocator->getNumChannels(), 2);
    EXPECT_EQ(allocator->getTotalBlocks(), 16);
}

TEST_F(AudioBlockAllocatorTest, CanAllocateBlock) {
    float* block = allocator->allocate();
    EXPECT_NE(block, nullptr);
    EXPECT_EQ(allocator->getAvailableBlocks(), 15);
}

TEST_F(AudioBlockAllocatorTest, CanDeallocateBlock) {
    float* block = allocator->allocate();
    allocator->deallocate(block);
    EXPECT_EQ(allocator->getAvailableBlocks(), 16);
}

TEST_F(AudioBlockAllocatorTest, ResetReturnsAllBlocks) {
    allocator->allocate();
    allocator->allocate();
    allocator->reset();
    EXPECT_EQ(allocator->getAvailableBlocks(), 16);
}

TEST_F(AudioBlockAllocatorTest, ScopedBlockWorks) {
    {
        ScopedAudioBlock block(*allocator);
        EXPECT_TRUE(static_cast<bool>(block));
        EXPECT_EQ(allocator->getAvailableBlocks(), 15);
    }
    EXPECT_EQ(allocator->getAvailableBlocks(), 16);
}

} // namespace test
} // namespace nap
