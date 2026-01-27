#ifndef NAP_BIQUADFILTERFACTORY_H
#define NAP_BIQUADFILTERFACTORY_H

#include "BiQuadFilter.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating BiQuadFilter instances.
 */
class BiQuadFilterFactory {
public:
    static std::unique_ptr<BiQuadFilter> create()
    {
        return std::make_unique<BiQuadFilter>();
    }

    static std::unique_ptr<BiQuadFilter> createLowPass(float frequency, float q = 0.707f)
    {
        auto node = std::make_unique<BiQuadFilter>();
        node->setFilterType(BiQuadFilter::FilterType::LowPass);
        node->setFrequency(frequency);
        node->setQ(q);
        return node;
    }

    static std::unique_ptr<BiQuadFilter> createHighPass(float frequency, float q = 0.707f)
    {
        auto node = std::make_unique<BiQuadFilter>();
        node->setFilterType(BiQuadFilter::FilterType::HighPass);
        node->setFrequency(frequency);
        node->setQ(q);
        return node;
    }

    static std::unique_ptr<BiQuadFilter> createBandPass(float frequency, float q)
    {
        auto node = std::make_unique<BiQuadFilter>();
        node->setFilterType(BiQuadFilter::FilterType::BandPass);
        node->setFrequency(frequency);
        node->setQ(q);
        return node;
    }
};

} // namespace nap

#endif // NAP_BIQUADFILTERFACTORY_H
