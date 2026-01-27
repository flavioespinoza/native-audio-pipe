#ifndef NAP_MIXERNODEFACTORY_H
#define NAP_MIXERNODEFACTORY_H

#include "MixerNode.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating MixerNode instances.
 */
class MixerNodeFactory {
public:
    /**
     * @brief Create a new MixerNode instance with default inputs.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<MixerNode> create()
    {
        return std::make_unique<MixerNode>();
    }

    /**
     * @brief Create a new MixerNode with specified number of inputs.
     * @param numInputs Number of input channels
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<MixerNode> create(std::uint32_t numInputs)
    {
        return std::make_unique<MixerNode>(numInputs);
    }

    /**
     * @brief Create a stereo mixer (2 inputs).
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<MixerNode> createStereo()
    {
        return std::make_unique<MixerNode>(2);
    }

    /**
     * @brief Create a quad mixer (4 inputs).
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<MixerNode> createQuad()
    {
        return std::make_unique<MixerNode>(4);
    }
};

} // namespace nap

#endif // NAP_MIXERNODEFACTORY_H
