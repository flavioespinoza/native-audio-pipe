#ifndef NAP_CHORUSFACTORY_H
#define NAP_CHORUSFACTORY_H

#include "Chorus.h"
#include <memory>

namespace nap {

class ChorusFactory {
public:
    static std::unique_ptr<Chorus> create()
    {
        return std::make_unique<Chorus>();
    }

    static std::unique_ptr<Chorus> create(float rate, float depth, std::uint32_t voices)
    {
        auto node = std::make_unique<Chorus>();
        node->setRate(rate);
        node->setDepth(depth);
        node->setVoices(voices);
        return node;
    }
};

} // namespace nap

#endif // NAP_CHORUSFACTORY_H
