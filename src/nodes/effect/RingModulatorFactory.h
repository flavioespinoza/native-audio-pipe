#ifndef NAP_RINGMODULATORFACTORY_H
#define NAP_RINGMODULATORFACTORY_H

#include "RingModulator.h"
#include <memory>

namespace nap {

class RingModulatorFactory {
public:
    static std::unique_ptr<RingModulator> create()
    {
        return std::make_unique<RingModulator>();
    }

    static std::unique_ptr<RingModulator> create(float frequency)
    {
        auto node = std::make_unique<RingModulator>();
        node->setFrequency(frequency);
        return node;
    }
};

} // namespace nap

#endif // NAP_RINGMODULATORFACTORY_H
