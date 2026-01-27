#ifndef NAP_GAINNODEFACTORY_H
#define NAP_GAINNODEFACTORY_H

#include "GainNode.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating GainNode instances.
 */
class GainNodeFactory {
public:
    /**
     * @brief Create a new GainNode instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<GainNode> create()
    {
        return std::make_unique<GainNode>();
    }

    /**
     * @brief Create a new GainNode with initial gain.
     * @param initialGain Initial gain value (linear)
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<GainNode> create(float initialGain)
    {
        auto node = std::make_unique<GainNode>();
        node->setGain(initialGain);
        return node;
    }

    /**
     * @brief Create a new GainNode with initial gain in dB.
     * @param initialGainDb Initial gain value in decibels
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<GainNode> createWithDb(float initialGainDb)
    {
        auto node = std::make_unique<GainNode>();
        node->setGainDb(initialGainDb);
        return node;
    }
};

} // namespace nap

#endif // NAP_GAINNODEFACTORY_H
