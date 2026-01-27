#include <gtest/gtest.h>
#include "core/serialization/StateVector.h"

namespace nap {
namespace test {

class StateVectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        vec = std::make_unique<StateVector>();
    }

    std::unique_ptr<StateVector> vec;
};

TEST_F(StateVectorTest, DefaultConstruction) {
    EXPECT_EQ(vec->size(), 0u);
    EXPECT_TRUE(vec->empty());
}

TEST_F(StateVectorTest, CapacityConstruction) {
    StateVector capacityVec(100);
    EXPECT_EQ(capacityVec.size(), 0u);
    EXPECT_GE(capacityVec.capacity(), 100u);
}

TEST_F(StateVectorTest, PushFloat) {
    vec->pushFloat(1.0f);
    vec->pushFloat(2.0f);
    vec->pushFloat(3.0f);

    EXPECT_EQ(vec->size(), 3u);
    EXPECT_FLOAT_EQ(vec->getFloat(0), 1.0f);
    EXPECT_FLOAT_EQ(vec->getFloat(1), 2.0f);
    EXPECT_FLOAT_EQ(vec->getFloat(2), 3.0f);
}

TEST_F(StateVectorTest, SetFloat) {
    vec->resize(3);
    vec->setFloat(0, 1.0f);
    vec->setFloat(1, 2.0f);
    vec->setFloat(2, 3.0f);

    EXPECT_FLOAT_EQ((*vec)[0], 1.0f);
    EXPECT_FLOAT_EQ((*vec)[1], 2.0f);
    EXPECT_FLOAT_EQ((*vec)[2], 3.0f);
}

TEST_F(StateVectorTest, PushAndGetInt) {
    vec->pushInt(42);
    vec->pushInt(-100);

    EXPECT_EQ(vec->getInt(0), 42);
    EXPECT_EQ(vec->getInt(1), -100);
}

TEST_F(StateVectorTest, Reserve) {
    vec->reserve(1000);
    EXPECT_GE(vec->capacity(), 1000u);
    EXPECT_EQ(vec->size(), 0u);
}

TEST_F(StateVectorTest, Resize) {
    vec->resize(5);
    EXPECT_EQ(vec->size(), 5u);

    // Should be zero-initialized
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_FLOAT_EQ(vec->getFloat(i), 0.0f);
    }
}

TEST_F(StateVectorTest, Clear) {
    vec->pushFloat(1.0f);
    vec->pushFloat(2.0f);
    vec->clear();

    EXPECT_EQ(vec->size(), 0u);
    EXPECT_TRUE(vec->empty());
}

TEST_F(StateVectorTest, DataAccess) {
    vec->pushFloat(1.0f);
    vec->pushFloat(2.0f);

    const float* data = vec->data();
    EXPECT_FLOAT_EQ(data[0], 1.0f);
    EXPECT_FLOAT_EQ(data[1], 2.0f);
}

TEST_F(StateVectorTest, Equals) {
    vec->pushFloat(1.0f);
    vec->pushFloat(2.0f);

    StateVector other;
    other.pushFloat(1.0f);
    other.pushFloat(2.0f);

    EXPECT_TRUE(vec->equals(other));
    EXPECT_TRUE(*vec == other);
}

TEST_F(StateVectorTest, EqualsWithTolerance) {
    vec->pushFloat(1.0f);

    StateVector other;
    other.pushFloat(1.0001f);

    EXPECT_FALSE(vec->equals(other, 0.00001f));
    EXPECT_TRUE(vec->equals(other, 0.001f));
}

TEST_F(StateVectorTest, NotEquals) {
    vec->pushFloat(1.0f);

    StateVector other;
    other.pushFloat(2.0f);

    EXPECT_TRUE(*vec != other);
}

TEST_F(StateVectorTest, Distance) {
    vec->pushFloat(0.0f);
    vec->pushFloat(0.0f);

    StateVector other;
    other.pushFloat(3.0f);
    other.pushFloat(4.0f);

    EXPECT_FLOAT_EQ(vec->distance(other), 5.0f);  // 3-4-5 triangle
}

TEST_F(StateVectorTest, Lerp) {
    vec->pushFloat(0.0f);
    vec->pushFloat(0.0f);

    StateVector b;
    b.pushFloat(10.0f);
    b.pushFloat(20.0f);

    auto result = StateVector::lerp(*vec, b, 0.5f);

    EXPECT_FLOAT_EQ(result.getFloat(0), 5.0f);
    EXPECT_FLOAT_EQ(result.getFloat(1), 10.0f);
}

TEST_F(StateVectorTest, ToAndFromBytes) {
    vec->pushFloat(1.5f);
    vec->pushFloat(2.5f);
    vec->pushFloat(3.5f);

    auto bytes = vec->toBytes();
    EXPECT_FALSE(bytes.empty());

    StateVector restored;
    EXPECT_TRUE(restored.fromBytes(bytes));

    EXPECT_EQ(restored.size(), 3u);
    EXPECT_FLOAT_EQ(restored.getFloat(0), 1.5f);
    EXPECT_FLOAT_EQ(restored.getFloat(1), 2.5f);
    EXPECT_FLOAT_EQ(restored.getFloat(2), 3.5f);
}

TEST_F(StateVectorTest, CopyConstruction) {
    vec->pushFloat(1.0f);
    vec->pushFloat(2.0f);

    StateVector copy(*vec);

    EXPECT_EQ(copy.size(), 2u);
    EXPECT_FLOAT_EQ(copy.getFloat(0), 1.0f);
    EXPECT_FLOAT_EQ(copy.getFloat(1), 2.0f);
}

TEST_F(StateVectorTest, CopyAssignment) {
    vec->pushFloat(1.0f);

    StateVector other;
    other = *vec;

    EXPECT_EQ(other.size(), 1u);
    EXPECT_FLOAT_EQ(other.getFloat(0), 1.0f);
}

} // namespace test
} // namespace nap
