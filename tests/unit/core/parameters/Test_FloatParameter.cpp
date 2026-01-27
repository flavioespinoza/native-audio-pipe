#include <gtest/gtest.h>
#include "core/parameters/FloatParameter.h"

namespace nap {
namespace test {

class FloatParameterTest : public ::testing::Test {
protected:
    void SetUp() override {
        param = std::make_unique<FloatParameter>("TestParam", 0.5f, 0.0f, 1.0f);
    }

    std::unique_ptr<FloatParameter> param;
};

TEST_F(FloatParameterTest, Construction) {
    EXPECT_EQ(param->getName(), "TestParam");
    EXPECT_EQ(param->getType(), ParameterType::Float);
    EXPECT_FLOAT_EQ(param->getValue(), 0.5f);
    EXPECT_FLOAT_EQ(param->getMinValue(), 0.0f);
    EXPECT_FLOAT_EQ(param->getMaxValue(), 1.0f);
}

TEST_F(FloatParameterTest, SetValue) {
    param->setValue(0.75f);
    EXPECT_FLOAT_EQ(param->getValue(), 0.75f);
}

TEST_F(FloatParameterTest, ClampToMin) {
    param->setValue(-0.5f);
    EXPECT_FLOAT_EQ(param->getValue(), 0.0f);
}

TEST_F(FloatParameterTest, ClampToMax) {
    param->setValue(1.5f);
    EXPECT_FLOAT_EQ(param->getValue(), 1.0f);
}

TEST_F(FloatParameterTest, NormalizedValue) {
    param->setValue(0.5f);
    EXPECT_FLOAT_EQ(param->getNormalizedValue(), 0.5f);

    param->setNormalizedValue(0.25f);
    EXPECT_FLOAT_EQ(param->getValue(), 0.25f);
}

TEST_F(FloatParameterTest, Reset) {
    param->setValue(0.9f);
    param->reset();
    EXPECT_FLOAT_EQ(param->getValue(), 0.5f);
}

TEST_F(FloatParameterTest, ChangeCallback) {
    float capturedOld = 0.0f;
    float capturedNew = 0.0f;
    int callCount = 0;

    param->setChangeCallback([&](float oldVal, float newVal) {
        capturedOld = oldVal;
        capturedNew = newVal;
        callCount++;
    });

    param->setValue(0.8f);
    EXPECT_EQ(callCount, 1);
    EXPECT_FLOAT_EQ(capturedOld, 0.5f);
    EXPECT_FLOAT_EQ(capturedNew, 0.8f);
}

TEST_F(FloatParameterTest, NoCallbackWhenValueUnchanged) {
    int callCount = 0;
    param->setChangeCallback([&](float, float) { callCount++; });

    param->setValue(0.5f);  // Same as default
    EXPECT_EQ(callCount, 0);
}

TEST_F(FloatParameterTest, SetRange) {
    param->setRange(-1.0f, 1.0f);
    EXPECT_FLOAT_EQ(param->getMinValue(), -1.0f);
    EXPECT_FLOAT_EQ(param->getMaxValue(), 1.0f);
    EXPECT_FLOAT_EQ(param->getValue(), 0.5f);  // Still in range
}

TEST_F(FloatParameterTest, Smoothing) {
    param->enableSmoothing(true, 0.01f);
    EXPECT_TRUE(param->isSmoothingEnabled());

    param->setValue(1.0f);
    float smoothed = param->getSmoothedValue(44100.0f);
    // Smoothed value should be between current and target
    EXPECT_GT(smoothed, 0.5f);
    EXPECT_LT(smoothed, 1.0f);
}

TEST_F(FloatParameterTest, CustomRange) {
    FloatParameter customParam("Frequency", 440.0f, 20.0f, 20000.0f);
    EXPECT_FLOAT_EQ(customParam.getValue(), 440.0f);
    EXPECT_FLOAT_EQ(customParam.getMinValue(), 20.0f);
    EXPECT_FLOAT_EQ(customParam.getMaxValue(), 20000.0f);
}

} // namespace test
} // namespace nap
