#ifndef NAP_SINEOSCILLATORFACTORY_H
#define NAP_SINEOSCILLATORFACTORY_H

#include "SineOscillator.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating SineOscillator instances.
 */
class SineOscillatorFactory {
public:
    /**
     * @brief Create a new SineOscillator instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SineOscillator> create()
    {
        return std::make_unique<SineOscillator>();
    }

    /**
     * @brief Create a new SineOscillator with specified frequency.
     * @param frequencyHz Frequency in Hertz
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SineOscillator> create(float frequencyHz)
    {
        auto node = std::make_unique<SineOscillator>();
        node->setFrequency(frequencyHz);
        return node;
    }

    /**
     * @brief Create a new SineOscillator with specified frequency and amplitude.
     * @param frequencyHz Frequency in Hertz
     * @param amplitude Amplitude (0.0 to 1.0)
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<SineOscillator> create(float frequencyHz, float amplitude)
    {
        auto node = std::make_unique<SineOscillator>();
        node->setFrequency(frequencyHz);
        node->setAmplitude(amplitude);
        return node;
    }
};

} // namespace nap

#endif // NAP_SINEOSCILLATORFACTORY_H
