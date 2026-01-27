#ifndef NAP_PHASERFACTORY_H
#define NAP_PHASERFACTORY_H

#include "Phaser.h"
#include <memory>

namespace nap {

class PhaserFactory {
public:
    static std::unique_ptr<Phaser> create()
    {
        return std::make_unique<Phaser>();
    }

    static std::unique_ptr<Phaser> create(float rate, float depth)
    {
        auto node = std::make_unique<Phaser>();
        node->setRate(rate);
        node->setDepth(depth);
        return node;
    }
};

} // namespace nap

#endif // NAP_PHASERFACTORY_H
