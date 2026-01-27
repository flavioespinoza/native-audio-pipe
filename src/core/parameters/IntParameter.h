#ifndef NAP_INT_PARAMETER_H
#define NAP_INT_PARAMETER_H

#include "api/IParameter.h"
#include <string>
#include <functional>

namespace nap {

/**
 * @brief Integer parameter with stepped values
 *
 * Handles discrete integer parameter values with range validation,
 * optional step size, and change notification callbacks.
 */
class IntParameter : public IParameter {
public:
    using ChangeCallback = std::function<void(int oldValue, int newValue)>;

    IntParameter(const std::string& name, int defaultValue,
                 int minValue = 0, int maxValue = 100, int stepSize = 1);
    ~IntParameter() override;

    // IParameter interface
    const std::string& getName() const override;
    ParameterType getType() const override;
    void reset() override;

    // Value access
    int getValue() const;
    void setValue(int value);

    // Normalized access (0.0 - 1.0)
    float getNormalizedValue() const;
    void setNormalizedValue(float normalized);

    // Range access
    int getMinValue() const;
    int getMaxValue() const;
    int getDefaultValue() const;
    int getStepSize() const;
    void setRange(int minValue, int maxValue);
    void setStepSize(int stepSize);

    // Step operations
    void increment();
    void decrement();

    // Callbacks
    void setChangeCallback(ChangeCallback callback);
    void removeChangeCallback();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_INT_PARAMETER_H
