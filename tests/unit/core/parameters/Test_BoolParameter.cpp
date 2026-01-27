#include <gtest/gtest.h>
#include "core/parameters/BoolParameter.h"

namespace nap {
namespace test {

class BoolParameterTest : public ::testing::Test {
protected:
    void SetUp() override {
        param = std::make_unique<BoolParameter>("TestParam", false);
    }

    std::unique_ptr<BoolParameter> param;
};

TEST_F(BoolParameterTest, Construction) {
    EXPECT_EQ(param->getName(), "TestParam");
    EXPECT_EQ(param->getType(), ParameterType::Bool);
    EXPECT_FALSE(param->getValue());
}

TEST_F(BoolParameterTest, SetValue) {
    param->setValue(true);
    EXPECT_TRUE(param->getValue());

    param->setValue(false);
    EXPECT_FALSE(param->getValue());
}

TEST_F(BoolParameterTest, Toggle) {
    EXPECT_FALSE(param->getValue());
    param->toggle();
    EXPECT_TRUE(param->getValue());
    param->toggle();
    EXPECT_FALSE(param->getValue());
}

TEST_F(BoolParameterTest, IsOnOff) {
    EXPECT_TRUE(param->isOff());
    EXPECT_FALSE(param->isOn());

    param->setValue(true);
    EXPECT_TRUE(param->isOn());
    EXPECT_FALSE(param->isOff());
}

TEST_F(BoolParameterTest, Reset) {
    param->setValue(true);
    param->reset();
    EXPECT_FALSE(param->getValue());
}

TEST_F(BoolParameterTest, DefaultTrue) {
    BoolParameter trueParam("Default True", true);
    EXPECT_TRUE(trueParam.getValue());
    EXPECT_TRUE(trueParam.getDefaultValue());

    trueParam.setValue(false);
    trueParam.reset();
    EXPECT_TRUE(trueParam.getValue());
}

TEST_F(BoolParameterTest, ChangeCallback) {
    bool capturedOld = true;
    bool capturedNew = true;
    int callCount = 0;

    param->setChangeCallback([&](bool oldVal, bool newVal) {
        capturedOld = oldVal;
        capturedNew = newVal;
        callCount++;
    });

    param->setValue(true);
    EXPECT_EQ(callCount, 1);
    EXPECT_FALSE(capturedOld);
    EXPECT_TRUE(capturedNew);
}

TEST_F(BoolParameterTest, NoCallbackWhenUnchanged) {
    int callCount = 0;
    param->setChangeCallback([&](bool, bool) { callCount++; });

    param->setValue(false);  // Already false
    EXPECT_EQ(callCount, 0);
}

} // namespace test
} // namespace nap
