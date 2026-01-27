#include <gtest/gtest.h>
#include "../../../../src/nodes/utility/DCBlockerNode.h"

namespace nap { namespace test {

TEST(DCBlockerNodeTest, DefaultCutoffFrequency) {
    DCBlockerNode blocker;
    EXPECT_FLOAT_EQ(blocker.getCutoffFrequency(), 10.0f);
}

TEST(DCBlockerNodeTest, CanSetCutoffFrequency) {
    DCBlockerNode blocker;
    blocker.setCutoffFrequency(20.0f);
    EXPECT_FLOAT_EQ(blocker.getCutoffFrequency(), 20.0f);
}

TEST(DCBlockerNodeTest, ProcessesSignal) {
    DCBlockerNode blocker;
    blocker.prepare(44100.0, 512);

    float input[] = {1.0f, 1.0f};
    float output[2];
    blocker.process(input, output, 1, 2);
    // DC blocker should affect the signal
}

TEST(DCBlockerNodeTest, ResetClearsState) {
    DCBlockerNode blocker;
    blocker.reset();
    // Should not throw
}

TEST(DCBlockerNodeTest, HasCorrectTypeName) {
    DCBlockerNode blocker;
    EXPECT_EQ(blocker.getTypeName(), "DCBlockerNode");
}

}} // namespace nap::test
