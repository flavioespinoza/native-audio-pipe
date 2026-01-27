#ifndef NAP_FLANGERFACTORY_H
#define NAP_FLANGERFACTORY_H

#include "Flanger.h"
#include <memory>

namespace nap {

class FlangerFactory {
public:
    static std::unique_ptr<Flanger> create()
    {
        return std::make_unique<Flanger>();
    }

    static std::unique_ptr<Flanger> create(float rate, float depth)
    {
        auto node = std::make_unique<Flanger>();
        node->setRate(rate);
        node->setDepth(depth);
        return node;
    }
};

} // namespace nap

#endif // NAP_FLANGERFACTORY_H
