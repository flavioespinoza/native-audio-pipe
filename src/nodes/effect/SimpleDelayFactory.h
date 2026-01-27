#ifndef NAP_SIMPLEDELAYFACTORY_H
#define NAP_SIMPLEDELAYFACTORY_H

#include "SimpleDelay.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating SimpleDelay instances.
 */
class SimpleDelayFactory {
public:
    static std::unique_ptr<SimpleDelay> create()
    {
        return std::make_unique<SimpleDelay>();
    }

    static std::unique_ptr<SimpleDelay> create(float delayTimeMs)
    {
        auto node = std::make_unique<SimpleDelay>();
        node->setDelayTime(delayTimeMs);
        return node;
    }

    static std::unique_ptr<SimpleDelay> create(float delayTimeMs, float feedback, float mix)
    {
        auto node = std::make_unique<SimpleDelay>();
        node->setDelayTime(delayTimeMs);
        node->setFeedback(feedback);
        node->setMix(mix);
        return node;
    }
};

} // namespace nap

#endif // NAP_SIMPLEDELAYFACTORY_H
