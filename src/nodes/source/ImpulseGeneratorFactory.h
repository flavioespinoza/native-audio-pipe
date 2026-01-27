#ifndef NAP_IMPULSEGENERATORFACTORY_H
#define NAP_IMPULSEGENERATORFACTORY_H

#include "ImpulseGenerator.h"
#include <memory>

namespace nap {

/**
 * @brief Factory for creating ImpulseGenerator instances.
 */
class ImpulseGeneratorFactory {
public:
    /**
     * @brief Create a new ImpulseGenerator instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<ImpulseGenerator> create()
    {
        return std::make_unique<ImpulseGenerator>();
    }

    /**
     * @brief Create a new ImpulseGenerator with specified interval.
     * @param intervalMs Interval between impulses in milliseconds
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<ImpulseGenerator> create(float intervalMs)
    {
        auto node = std::make_unique<ImpulseGenerator>();
        node->setIntervalMs(intervalMs);
        return node;
    }

    /**
     * @brief Create a one-shot ImpulseGenerator.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<ImpulseGenerator> createOneShot()
    {
        auto node = std::make_unique<ImpulseGenerator>();
        node->setOneShot(true);
        return node;
    }
};

} // namespace nap

#endif // NAP_IMPULSEGENERATORFACTORY_H
