#include <gtest/gtest.h>
#include "core/parameters/EnumParameter.h"

namespace nap {
namespace test {

class EnumParameterTest : public ::testing::Test {
protected:
    void SetUp() override {
        options = {"Option A", "Option B", "Option C"};
        param = std::make_unique<EnumParameter>("TestParam", options, 0);
    }

    std::vector<std::string> options;
    std::unique_ptr<EnumParameter> param;
};

TEST_F(EnumParameterTest, Construction) {
    EXPECT_EQ(param->getName(), "TestParam");
    EXPECT_EQ(param->getType(), ParameterType::Enum);
    EXPECT_EQ(param->getSelectedIndex(), 0u);
    EXPECT_EQ(param->getSelectedValue(), "Option A");
}

TEST_F(EnumParameterTest, SetSelectedIndex) {
    param->setSelectedIndex(1);
    EXPECT_EQ(param->getSelectedIndex(), 1u);
    EXPECT_EQ(param->getSelectedValue(), "Option B");
}

TEST_F(EnumParameterTest, ClampIndex) {
    param->setSelectedIndex(100);
    EXPECT_EQ(param->getSelectedIndex(), 2u);  // Clamped to last valid index
}

TEST_F(EnumParameterTest, SetSelectedValue) {
    EXPECT_TRUE(param->setSelectedValue("Option C"));
    EXPECT_EQ(param->getSelectedIndex(), 2u);

    EXPECT_FALSE(param->setSelectedValue("Invalid"));
    EXPECT_EQ(param->getSelectedIndex(), 2u);  // Unchanged
}

TEST_F(EnumParameterTest, GetOptions) {
    auto opts = param->getOptions();
    EXPECT_EQ(opts.size(), 3u);
    EXPECT_EQ(opts[0], "Option A");
    EXPECT_EQ(opts[1], "Option B");
    EXPECT_EQ(opts[2], "Option C");
}

TEST_F(EnumParameterTest, SelectNext) {
    EXPECT_EQ(param->getSelectedIndex(), 0u);
    param->selectNext();
    EXPECT_EQ(param->getSelectedIndex(), 1u);
    param->selectNext();
    EXPECT_EQ(param->getSelectedIndex(), 2u);
    param->selectNext();  // Wraps around
    EXPECT_EQ(param->getSelectedIndex(), 0u);
}

TEST_F(EnumParameterTest, SelectPrevious) {
    param->setSelectedIndex(2);
    param->selectPrevious();
    EXPECT_EQ(param->getSelectedIndex(), 1u);
    param->selectPrevious();
    EXPECT_EQ(param->getSelectedIndex(), 0u);
    param->selectPrevious();  // Wraps around
    EXPECT_EQ(param->getSelectedIndex(), 2u);
}

TEST_F(EnumParameterTest, Reset) {
    param->setSelectedIndex(2);
    param->reset();
    EXPECT_EQ(param->getSelectedIndex(), 0u);
}

TEST_F(EnumParameterTest, ChangeCallback) {
    size_t capturedOld = 999;
    size_t capturedNew = 999;
    int callCount = 0;

    param->setChangeCallback([&](size_t oldIdx, size_t newIdx) {
        capturedOld = oldIdx;
        capturedNew = newIdx;
        callCount++;
    });

    param->setSelectedIndex(2);
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(capturedOld, 0u);
    EXPECT_EQ(capturedNew, 2u);
}

TEST_F(EnumParameterTest, AddOption) {
    param->addOption("Option D");
    EXPECT_EQ(param->getOptionCount(), 4u);
}

TEST_F(EnumParameterTest, SetOptions) {
    param->setSelectedIndex(2);
    param->setOptions({"New A", "New B"});
    EXPECT_EQ(param->getOptionCount(), 2u);
    EXPECT_EQ(param->getSelectedIndex(), 1u);  // Clamped
}

} // namespace test
} // namespace nap
