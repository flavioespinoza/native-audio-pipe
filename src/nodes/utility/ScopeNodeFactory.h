#ifndef NAP_SCOPENODEFACTORY_H
#define NAP_SCOPENODEFACTORY_H

#include "ScopeNode.h"
#include <memory>

namespace nap {

class ScopeNodeFactory {
public:
    static std::unique_ptr<ScopeNode> create()
    {
        return std::make_unique<ScopeNode>();
    }

    static std::unique_ptr<ScopeNode> create(std::size_t bufferSize)
    {
        auto node = std::make_unique<ScopeNode>();
        node->setBufferSize(bufferSize);
        return node;
    }
};

} // namespace nap

#endif // NAP_SCOPENODEFACTORY_H
