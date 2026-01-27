#include <gtest/gtest.h>
#include "../../../../src/nodes/utility/ScopeNode.h"

namespace nap { namespace test {

TEST(ScopeNodeTest, DefaultBufferSize) {
    ScopeNode scope;
    EXPECT_EQ(scope.getBufferSize(), 1024);
}

TEST(ScopeNodeTest, CanSetBufferSize) {
    ScopeNode scope;
    scope.setBufferSize(2048);
    EXPECT_EQ(scope.getBufferSize(), 2048);
}

TEST(ScopeNodeTest, DefaultTriggerIsDisabled) {
    ScopeNode scope;
    EXPECT_FALSE(scope.isTriggerEnabled());
}

TEST(ScopeNodeTest, CanEnableTrigger) {
    ScopeNode scope;
    scope.setTriggerEnabled(true);
    EXPECT_TRUE(scope.isTriggerEnabled());
}

TEST(ScopeNodeTest, CanGetWaveformData) {
    ScopeNode scope;
    scope.setBufferSize(64);
    auto data = scope.getWaveformData(0);
    EXPECT_EQ(data.size(), 64);
}

TEST(ScopeNodeTest, HasCorrectTypeName) {
    ScopeNode scope;
    EXPECT_EQ(scope.getTypeName(), "ScopeNode");
}

}} // namespace nap::test
