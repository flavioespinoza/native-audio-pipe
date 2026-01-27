#ifndef NAP_SUMMERNODEFACTORY_H
#define NAP_SUMMERNODEFACTORY_H

#include "SummerNode.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating SummerNode instances.
 */
class SummerNodeFactory {
public:
    /**
     * @brief Create a new SummerNode instance with default inputs.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SummerNode> create()
    {
        return std::make_unique<SummerNode>();
    }

    /**
     * @brief Create a new SummerNode with specified number of inputs.
     * @param numInputs Number of input streams to sum
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SummerNode> create(std::uint32_t numInputs)
    {
        return std::make_unique<SummerNode>(numInputs);
    }

    /**
     * @brief Create a normalized summer (output divided by number of inputs).
     * @param numInputs Number of input streams to sum
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SummerNode> createNormalized(std::uint32_t numInputs)
    {
        auto node = std::make_unique<SummerNode>(numInputs);
        node->setNormalize(true);
        return node;
    }
};

} // namespace nap

#endif // NAP_SUMMERNODEFACTORY_H
