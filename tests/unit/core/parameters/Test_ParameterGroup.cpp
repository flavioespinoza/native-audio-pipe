#include <gtest/gtest.h>
#include "core/parameters/ParameterGroup.h"
#include "core/parameters/FloatParameter.h"
#include "core/parameters/BoolParameter.h"

namespace nap {
namespace test {

class ParameterGroupTest : public ::testing::Test {
protected:
    void SetUp() override {
        group = std::make_unique<ParameterGroup>("TestGroup");
    }

    std::unique_ptr<ParameterGroup> group;
};

TEST_F(ParameterGroupTest, Construction) {
    EXPECT_EQ(group->getName(), "TestGroup");
    EXPECT_EQ(group->getParameterCount(), 0u);
    EXPECT_EQ(group->getGroupCount(), 0u);
}

TEST_F(ParameterGroupTest, SetName) {
    group->setName("NewName");
    EXPECT_EQ(group->getName(), "NewName");
}

TEST_F(ParameterGroupTest, AddParameter) {
    auto param = std::make_shared<FloatParameter>("Gain", 0.5f);
    group->addParameter(param);

    EXPECT_EQ(group->getParameterCount(), 1u);
    EXPECT_TRUE(group->hasParameter("Gain"));
}

TEST_F(ParameterGroupTest, GetParameter) {
    auto param = std::make_shared<FloatParameter>("Gain", 0.5f);
    group->addParameter(param);

    auto* retrieved = group->getParameter("Gain");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "Gain");
}

TEST_F(ParameterGroupTest, GetNonexistentParameter) {
    EXPECT_EQ(group->getParameter("Nonexistent"), nullptr);
}

TEST_F(ParameterGroupTest, RemoveParameter) {
    auto param = std::make_shared<FloatParameter>("Gain", 0.5f);
    group->addParameter(param);

    group->removeParameter("Gain");
    EXPECT_EQ(group->getParameterCount(), 0u);
    EXPECT_FALSE(group->hasParameter("Gain"));
}

TEST_F(ParameterGroupTest, AddNestedGroup) {
    auto subGroup = std::make_unique<ParameterGroup>("SubGroup");
    group->addGroup(std::move(subGroup));

    EXPECT_EQ(group->getGroupCount(), 1u);
    EXPECT_TRUE(group->hasGroup("SubGroup"));
}

TEST_F(ParameterGroupTest, GetNestedGroup) {
    auto subGroup = std::make_unique<ParameterGroup>("SubGroup");
    group->addGroup(std::move(subGroup));

    auto* retrieved = group->getGroup("SubGroup");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "SubGroup");
}

TEST_F(ParameterGroupTest, RemoveNestedGroup) {
    auto subGroup = std::make_unique<ParameterGroup>("SubGroup");
    group->addGroup(std::move(subGroup));

    group->removeGroup("SubGroup");
    EXPECT_EQ(group->getGroupCount(), 0u);
    EXPECT_FALSE(group->hasGroup("SubGroup"));
}

TEST_F(ParameterGroupTest, ForEachParameter) {
    group->addParameter(std::make_shared<FloatParameter>("Param1", 0.0f));
    group->addParameter(std::make_shared<FloatParameter>("Param2", 0.5f));
    group->addParameter(std::make_shared<FloatParameter>("Param3", 1.0f));

    int count = 0;
    group->forEachParameter([&](const IParameter&) { count++; });
    EXPECT_EQ(count, 3);
}

TEST_F(ParameterGroupTest, ResetAll) {
    auto param1 = std::make_shared<FloatParameter>("Gain", 0.5f, 0.0f, 1.0f);
    auto param2 = std::make_shared<BoolParameter>("Bypass", false);

    param1->setValue(1.0f);
    param2->setValue(true);

    group->addParameter(param1);
    group->addParameter(param2);

    group->resetAll();

    EXPECT_FLOAT_EQ(static_cast<FloatParameter*>(group->getParameter("Gain"))->getValue(), 0.5f);
    EXPECT_FALSE(static_cast<BoolParameter*>(group->getParameter("Bypass"))->getValue());
}

TEST_F(ParameterGroupTest, Clear) {
    group->addParameter(std::make_shared<FloatParameter>("Param", 0.0f));
    auto subGroup = std::make_unique<ParameterGroup>("SubGroup");
    group->addGroup(std::move(subGroup));

    group->clear();

    EXPECT_EQ(group->getParameterCount(), 0u);
    EXPECT_EQ(group->getGroupCount(), 0u);
}

TEST_F(ParameterGroupTest, DuplicateParameterIgnored) {
    auto param1 = std::make_shared<FloatParameter>("Gain", 0.5f);
    auto param2 = std::make_shared<FloatParameter>("Gain", 1.0f);

    group->addParameter(param1);
    group->addParameter(param2);

    EXPECT_EQ(group->getParameterCount(), 1u);
}

} // namespace test
} // namespace nap
