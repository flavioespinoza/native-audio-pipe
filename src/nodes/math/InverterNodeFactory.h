#ifndef NAP_INVERTERNODEFACTORY_H
#define NAP_INVERTERNODEFACTORY_H

#include "InverterNode.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating InverterNode instances.
 */
class InverterNodeFactory {
public:
    /**
     * @brief Create a new InverterNode instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<InverterNode> create()
    {
        return std::make_unique<InverterNode>();
    }

    /**
     * @brief Create a new InverterNode that inverts left channel only.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<InverterNode> createLeftOnly()
    {
        auto node = std::make_unique<InverterNode>();
        node->setInvertLeft(true);
        node->setInvertRight(false);
        return node;
    }

    /**
     * @brief Create a new InverterNode that inverts right channel only.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<InverterNode> createRightOnly()
    {
        auto node = std::make_unique<InverterNode>();
        node->setInvertLeft(false);
        node->setInvertRight(true);
        return node;
    }
};

} // namespace nap

#endif // NAP_INVERTERNODEFACTORY_H
