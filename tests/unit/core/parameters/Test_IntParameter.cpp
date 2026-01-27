#include <gtest/gtest.h>
#include "core/parameters/IntParameter.h"

namespace nap {
namespace test {

class IntParameterTest : public ::testing::Test {
protected:
    void SetUp() override {
        param = std::make_unique<IntParameter>("TestParam", 50, 0, 100, 1);
    }

    std::unique_ptr<IntParameter> param;
};

TEST_F(IntParameterTest, Construction) {
    EXPECT_EQ(param->getName(), "TestParam");
    EXPECT_EQ(param->getType(), ParameterType::Int);
    EXPECT_EQ(param->getValue(), 50);
    EXPECT_EQ(param->getMinValue(), 0);
    EXPECT_EQ(param->getMaxValue(), 100);
}

TEST_F(IntParameterTest, SetValue) {
    param->setValue(75);
    EXPECT_EQ(param->getValue(), 75);
}

TEST_F(IntParameterTest, ClampToMin) {
    param->setValue(-10);
    EXPECT_EQ(param->getValue(), 0);
}

TEST_F(IntParameterTest, ClampToMax) {
    param->setValue(150);
    EXPECT_EQ(param->getValue(), 100);
}

TEST_F(IntParameterTest, NormalizedValue) {
    param->setValue(50);
    EXPECT_FLOAT_EQ(param->getNormalizedValue(), 0.5f);

    param->setNormalizedValue(0.25f);
    EXPECT_EQ(param->getValue(), 25);
}

TEST_F(IntParameterTest, Reset) {
    param->setValue(90);
    param->reset();
    EXPECT_EQ(param->getValue(), 50);
}

TEST_F(IntParameterTest, Increment) {
    param->setValue(50);
    param->increment();
    EXPECT_EQ(param->getValue(), 51);
}

TEST_F(IntParameterTest, Decrement) {
    param->setValue(50);
    param->decrement();
    EXPECT_EQ(param->getValue(), 49);
}

TEST_F(IntParameterTest, StepSize) {
    IntParameter steppedParam("Stepped", 0, 0, 100, 10);
    steppedParam.setValue(5);
    EXPECT_EQ(steppedParam.getValue(), 0);  // Quantized

    steppedParam.setValue(15);
    EXPECT_EQ(steppedParam.getValue(), 10);
}

TEST_F(IntParameterTest, IncrementWithStep) {
    IntParameter steppedParam("Stepped", 0, 0, 100, 10);
    steppedParam.increment();
    EXPECT_EQ(steppedParam.getValue(), 10);
}

TEST_F(IntParameterTest, ChangeCallback) {
    int capturedOld = 0;
    int capturedNew = 0;
    int callCount = 0;

    param->setChangeCallback([&](int oldVal, int newVal) {
        capturedOld = oldVal;
        capturedNew = newVal;
        callCount++;
    });

    param->setValue(80);
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(capturedOld, 50);
    EXPECT_EQ(capturedNew, 80);
}

} // namespace test
} // namespace nap
