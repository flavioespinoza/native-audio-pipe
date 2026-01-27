#ifndef NAP_HARDCLIPPERFACTORY_H
#define NAP_HARDCLIPPERFACTORY_H

#include "HardClipper.h"
#include <memory>

namespace nap {

class HardClipperFactory {
public:
    static std::unique_ptr<HardClipper> create()
    {
        return std::make_unique<HardClipper>();
    }

    static std::unique_ptr<HardClipper> create(float threshold)
    {
        auto node = std::make_unique<HardClipper>();
        node->setThreshold(threshold);
        return node;
    }
};

} // namespace nap

#endif // NAP_HARDCLIPPERFACTORY_H
