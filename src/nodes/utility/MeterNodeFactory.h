#ifndef NAP_METERNODEFACTORY_H
#define NAP_METERNODEFACTORY_H

#include "MeterNode.h"
#include <memory>

namespace nap {

class MeterNodeFactory {
public:
    static std::unique_ptr<MeterNode> create()
    {
        return std::make_unique<MeterNode>();
    }
};

} // namespace nap

#endif // NAP_METERNODEFACTORY_H
