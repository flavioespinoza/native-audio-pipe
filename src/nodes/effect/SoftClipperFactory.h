#ifndef NAP_SOFTCLIPPERFACTORY_H
#define NAP_SOFTCLIPPERFACTORY_H

#include "SoftClipper.h"
#include <memory>

namespace nap {

class SoftClipperFactory {
public:
    static std::unique_ptr<SoftClipper> create()
    {
        return std::make_unique<SoftClipper>();
    }

    static std::unique_ptr<SoftClipper> create(float drive)
    {
        auto node = std::make_unique<SoftClipper>();
        node->setDrive(drive);
        return node;
    }

    static std::unique_ptr<SoftClipper> create(float drive, SoftClipper::CurveType curveType)
    {
        auto node = std::make_unique<SoftClipper>();
        node->setDrive(drive);
        node->setCurveType(curveType);
        return node;
    }
};

} // namespace nap

#endif // NAP_SOFTCLIPPERFACTORY_H
