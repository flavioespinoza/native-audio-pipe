#ifndef NAP_SPLITTERNODEFACTORY_H
#define NAP_SPLITTERNODEFACTORY_H

#include "SplitterNode.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating SplitterNode instances.
 */
class SplitterNodeFactory {
public:
    /**
     * @brief Create a new SplitterNode instance with default outputs.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SplitterNode> create()
    {
        return std::make_unique<SplitterNode>();
    }

    /**
     * @brief Create a new SplitterNode with specified number of outputs.
     * @param numOutputs Number of output channels
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SplitterNode> create(std::uint32_t numOutputs)
    {
        return std::make_unique<SplitterNode>(numOutputs);
    }

    /**
     * @brief Create a stereo splitter (2 outputs).
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SplitterNode> createStereo()
    {
        return std::make_unique<SplitterNode>(2);
    }

    /**
     * @brief Create a quad splitter (4 outputs).
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SplitterNode> createQuad()
    {
        return std::make_unique<SplitterNode>(4);
    }
};

} // namespace nap

#endif // NAP_SPLITTERNODEFACTORY_H
