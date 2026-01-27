#ifndef NAP_WHITENOISEFACTORY_H
#define NAP_WHITENOISEFACTORY_H

#include "WhiteNoise.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating WhiteNoise instances.
 */
class WhiteNoiseFactory {
public:
    /**
     * @brief Create a new WhiteNoise instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<WhiteNoise> create()
    {
        return std::make_unique<WhiteNoise>();
    }

    /**
     * @brief Create a new WhiteNoise with specified amplitude.
     * @param amplitude Amplitude (0.0 to 1.0)
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<WhiteNoise> create(float amplitude)
    {
        auto node = std::make_unique<WhiteNoise>();
        node->setAmplitude(amplitude);
        return node;
    }
};

} // namespace nap

#endif // NAP_WHITENOISEFACTORY_H
