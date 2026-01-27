#ifndef NAP_DCBLOCKERNODEFACTORY_H
#define NAP_DCBLOCKERNODEFACTORY_H

#include "DCBlockerNode.h"
#include <memory>

namespace nap {

class DCBlockerNodeFactory {
public:
    static std::unique_ptr<DCBlockerNode> create()
    {
        return std::make_unique<DCBlockerNode>();
    }

    static std::unique_ptr<DCBlockerNode> create(float cutoffHz)
    {
        auto node = std::make_unique<DCBlockerNode>();
        node->setCutoffFrequency(cutoffHz);
        return node;
    }
};

} // namespace nap

#endif // NAP_DCBLOCKERNODEFACTORY_H
