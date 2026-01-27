#ifndef NAP_PINKNOISEFACTORY_H
#define NAP_PINKNOISEFACTORY_H

#include "PinkNoise.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating PinkNoise instances.
 */
class PinkNoiseFactory {
public:
    /**
     * @brief Create a new PinkNoise instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<PinkNoise> create()
    {
        return std::make_unique<PinkNoise>();
    }

    /**
     * @brief Create a new PinkNoise with specified amplitude.
     * @param amplitude Amplitude (0.0 to 1.0)
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<PinkNoise> create(float amplitude)
    {
        auto node = std::make_unique<PinkNoise>();
        node->setAmplitude(amplitude);
        return node;
    }
};

} // namespace nap

#endif // NAP_PINKNOISEFACTORY_H
