#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/BiQuadFilter.h"

namespace nap { namespace test {

TEST(BiQuadFilterTest, DefaultFilterTypeIsLowPass) {
    BiQuadFilter filter;
    EXPECT_EQ(filter.getFilterType(), BiQuadFilter::FilterType::LowPass);
}

TEST(BiQuadFilterTest, CanSetFilterType) {
    BiQuadFilter filter;
    filter.setFilterType(BiQuadFilter::FilterType::HighPass);
    EXPECT_EQ(filter.getFilterType(), BiQuadFilter::FilterType::HighPass);
}

TEST(BiQuadFilterTest, CanSetFrequency) {
    BiQuadFilter filter;
    filter.setFrequency(2000.0f);
    EXPECT_FLOAT_EQ(filter.getFrequency(), 2000.0f);
}

TEST(BiQuadFilterTest, CanSetQ) {
    BiQuadFilter filter;
    filter.setQ(1.5f);
    EXPECT_FLOAT_EQ(filter.getQ(), 1.5f);
}

TEST(BiQuadFilterTest, HasCorrectTypeName) {
    BiQuadFilter filter;
    EXPECT_EQ(filter.getTypeName(), "BiQuadFilter");
}

}} // namespace nap::test
