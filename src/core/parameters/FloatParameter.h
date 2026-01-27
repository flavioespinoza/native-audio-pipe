#ifndef NAP_FLOAT_PARAMETER_H
#define NAP_FLOAT_PARAMETER_H

#include "api/IParameter.h"
#include <string>
#include <functional>

namespace nap {

/**
 * @brief Float parameter with continuous values and min/max clamping
 *
 * Handles floating-point parameter values with range validation,
 * smoothing support, and change notification callbacks.
 */
class FloatParameter : public IParameter {
public:
    using ChangeCallback = std::function<void(float oldValue, float newValue)>;

    FloatParameter(const std::string& name, float defaultValue,
                   float minValue = 0.0f, float maxValue = 1.0f);
    ~FloatParameter() override;

    // IParameter interface
    const std::string& getName() const override;
    ParameterType getType() const override;
    void reset() override;

    // Value access
    float getValue() const;
    void setValue(float value);

    // Normalized access (0.0 - 1.0)
    float getNormalizedValue() const;
    void setNormalizedValue(float normalized);

    // Range access
    float getMinValue() const;
    float getMaxValue() const;
    float getDefaultValue() const;
    void setRange(float minValue, float maxValue);

    // Smoothing
    void enableSmoothing(bool enable, float smoothingTime = 0.01f);
    bool isSmoothingEnabled() const;
    float getSmoothedValue(float sampleRate);

    // Callbacks
    void setChangeCallback(ChangeCallback callback);
    void removeChangeCallback();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_FLOAT_PARAMETER_H
