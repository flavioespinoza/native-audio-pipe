#ifndef NAP_BITCRUSHERFACTORY_H
#define NAP_BITCRUSHERFACTORY_H

#include "BitCrusher.h"
#include <memory>

namespace nap {

class BitCrusherFactory {
public:
    static std::unique_ptr<BitCrusher> create()
    {
        return std::make_unique<BitCrusher>();
    }

    static std::unique_ptr<BitCrusher> create(std::uint32_t bitDepth)
    {
        auto node = std::make_unique<BitCrusher>();
        node->setBitDepth(bitDepth);
        return node;
    }

    static std::unique_ptr<BitCrusher> create(std::uint32_t bitDepth, std::uint32_t downsampleFactor)
    {
        auto node = std::make_unique<BitCrusher>();
        node->setBitDepth(bitDepth);
        node->setDownsampleFactor(downsampleFactor);
        return node;
    }
};

} // namespace nap

#endif // NAP_BITCRUSHERFACTORY_H
