#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/ReverbConvolution.h"

namespace nap { namespace test {

TEST(ReverbConvolutionTest, InitiallyNoIRLoaded) {
    ReverbConvolution reverb;
    EXPECT_FALSE(reverb.isImpulseResponseLoaded());
}

TEST(ReverbConvolutionTest, CanUnloadIR) {
    ReverbConvolution reverb;
    reverb.unloadImpulseResponse();
    EXPECT_FALSE(reverb.isImpulseResponseLoaded());
}

TEST(ReverbConvolutionTest, DefaultDryWetMix) {
    ReverbConvolution reverb;
    EXPECT_FLOAT_EQ(reverb.getDryWetMix(), 0.5f);
}

TEST(ReverbConvolutionTest, CanSetDryWetMix) {
    ReverbConvolution reverb;
    reverb.setDryWetMix(0.7f);
    EXPECT_FLOAT_EQ(reverb.getDryWetMix(), 0.7f);
}

TEST(ReverbConvolutionTest, CanLoadIRFromBuffer) {
    ReverbConvolution reverb;
    float ir[] = {1.0f, 0.5f, 0.25f, 0.125f};
    EXPECT_TRUE(reverb.loadImpulseResponse(ir, 4));
    EXPECT_TRUE(reverb.isImpulseResponseLoaded());
    EXPECT_EQ(reverb.getImpulseResponseLength(), 4);
}

TEST(ReverbConvolutionTest, HasCorrectTypeName) {
    ReverbConvolution reverb;
    EXPECT_EQ(reverb.getTypeName(), "ReverbConvolution");
}

}} // namespace nap::test
