#include <gtest/gtest.h>
#include "core/parameters/TriggerParameter.h"

namespace nap {
namespace test {

class TriggerParameterTest : public ::testing::Test {
protected:
    void SetUp() override {
        param = std::make_unique<TriggerParameter>("TestTrigger");
    }

    std::unique_ptr<TriggerParameter> param;
};

TEST_F(TriggerParameterTest, Construction) {
    EXPECT_EQ(param->getName(), "TestTrigger");
    EXPECT_EQ(param->getType(), ParameterType::Trigger);
    EXPECT_FALSE(param->isPending());
}

TEST_F(TriggerParameterTest, Trigger) {
    EXPECT_FALSE(param->isPending());
    param->trigger();
    EXPECT_TRUE(param->isPending());
}

TEST_F(TriggerParameterTest, Consume) {
    param->trigger();
    EXPECT_TRUE(param->consume());
    EXPECT_FALSE(param->isPending());
    EXPECT_FALSE(param->consume());  // Already consumed
}

TEST_F(TriggerParameterTest, Reset) {
    param->trigger();
    param->reset();
    EXPECT_FALSE(param->isPending());
}

TEST_F(TriggerParameterTest, TriggerCallback) {
    int callCount = 0;
    param->setTriggerCallback([&]() { callCount++; });

    param->trigger();
    EXPECT_EQ(callCount, 1);

    param->trigger();
    EXPECT_EQ(callCount, 2);
}

TEST_F(TriggerParameterTest, RemoveCallback) {
    int callCount = 0;
    param->setTriggerCallback([&]() { callCount++; });

    param->trigger();
    EXPECT_EQ(callCount, 1);

    param->removeTriggerCallback();
    param->trigger();
    EXPECT_EQ(callCount, 1);  // Callback not called
}

TEST_F(TriggerParameterTest, MultipleTriggersBeforeConsume) {
    param->trigger();
    param->trigger();
    param->trigger();

    EXPECT_TRUE(param->isPending());
    EXPECT_TRUE(param->consume());
    EXPECT_FALSE(param->consume());
}

} // namespace test
} // namespace nap
