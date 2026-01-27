#ifndef NAP_PANNODEFACTORY_H
#define NAP_PANNODEFACTORY_H

#include "PanNode.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating PanNode instances.
 */
class PanNodeFactory {
public:
    /**
     * @brief Create a new PanNode instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<PanNode> create()
    {
        return std::make_unique<PanNode>();
    }

    /**
     * @brief Create a new PanNode with initial pan position.
     * @param initialPan Initial pan position (-1.0 to 1.0)
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<PanNode> create(float initialPan)
    {
        auto node = std::make_unique<PanNode>();
        node->setPan(initialPan);
        return node;
    }

    /**
     * @brief Create a new PanNode with specified pan law.
     * @param panLaw The pan law to use
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<PanNode> createWithLaw(PanNode::PanLaw panLaw)
    {
        auto node = std::make_unique<PanNode>();
        node->setPanLaw(panLaw);
        return node;
    }
};

} // namespace nap

#endif // NAP_PANNODEFACTORY_H
