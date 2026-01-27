#include <gtest/gtest.h>
#include "../../../../src/core/memory/CircularBuffer.h"

namespace nap {
namespace test {

class CircularBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer = std::make_unique<CircularBuffer>(1024);
    }

    std::unique_ptr<CircularBuffer> buffer;
};

TEST_F(CircularBufferTest, InitialStateIsEmpty) {
    EXPECT_TRUE(buffer->isEmpty());
    EXPECT_EQ(buffer->getAvailableForRead(), 0);
}

TEST_F(CircularBufferTest, CanWriteAndRead) {
    float data[] = {1.0f, 2.0f, 3.0f, 4.0f};
    EXPECT_EQ(buffer->write(data, 4), 4);
    EXPECT_EQ(buffer->getAvailableForRead(), 4);

    float output[4];
    EXPECT_EQ(buffer->read(output, 4), 4);
    EXPECT_FLOAT_EQ(output[0], 1.0f);
    EXPECT_FLOAT_EQ(output[3], 4.0f);
}

TEST_F(CircularBufferTest, ClearResetsBuffer) {
    float data[] = {1.0f, 2.0f, 3.0f};
    buffer->write(data, 3);
    buffer->clear();
    EXPECT_TRUE(buffer->isEmpty());
}

TEST_F(CircularBufferTest, PeekDoesNotRemoveData) {
    float data[] = {1.0f, 2.0f};
    buffer->write(data, 2);

    float output[2];
    buffer->peek(output, 2);
    EXPECT_EQ(buffer->getAvailableForRead(), 2);
}

} // namespace test
} // namespace nap
