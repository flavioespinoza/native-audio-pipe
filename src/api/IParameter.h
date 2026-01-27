#ifndef NAP_IPARAMETER_H
#define NAP_IPARAMETER_H

#include <cstdint>
#include <functional>
#include <string>

namespace nap {

/**
 * @brief Interface for audio node parameters with automation support.
 *
 * IParameter defines the contract for parameters that can be attached to audio nodes,
 * supporting real-time modulation, smoothing, and value change notifications.
 */
class IParameter {
public:
    virtual ~IParameter() = default;

    /**
     * @brief Get the parameter's unique identifier.
     * @return Parameter ID as a string
     */
    virtual std::string getParameterId() const = 0;

    /**
     * @brief Get the display name of the parameter.
     * @return Display name as a string
     */
    virtual std::string getDisplayName() const = 0;

    /**
     * @brief Get the current value of the parameter.
     * @return Current value as a float
     */
    virtual float getValue() const = 0;

    /**
     * @brief Set the parameter value.
     * @param value The new value to set
     */
    virtual void setValue(float value) = 0;

    /**
     * @brief Get the normalized value (0.0 to 1.0).
     * @return Normalized value
     */
    virtual float getNormalizedValue() const = 0;

    /**
     * @brief Set the parameter using a normalized value (0.0 to 1.0).
     * @param normalizedValue The normalized value to set
     */
    virtual void setNormalizedValue(float normalizedValue) = 0;

    /**
     * @brief Get the minimum allowed value.
     * @return Minimum value
     */
    virtual float getMinValue() const = 0;

    /**
     * @brief Get the maximum allowed value.
     * @return Maximum value
     */
    virtual float getMaxValue() const = 0;

    /**
     * @brief Get the default value.
     * @return Default value
     */
    virtual float getDefaultValue() const = 0;

    /**
     * @brief Reset the parameter to its default value.
     */
    virtual void resetToDefault() = 0;

    /**
     * @brief Get the smoothed value for real-time processing.
     * @return Smoothed value
     */
    virtual float getSmoothedValue() = 0;

    /**
     * @brief Set the smoothing time in milliseconds.
     * @param smoothingMs Smoothing time in milliseconds
     */
    virtual void setSmoothingTime(float smoothingMs) = 0;

    /**
     * @brief Get the unit label for display (e.g., "dB", "Hz", "%").
     * @return Unit label as a string
     */
    virtual std::string getUnitLabel() const = 0;

    using ValueChangedCallback = std::function<void(float oldValue, float newValue)>;

    /**
     * @brief Register a callback for value changes.
     * @param callback The callback function to register
     * @return Callback ID for later removal
     */
    virtual std::uint32_t addValueChangedCallback(ValueChangedCallback callback) = 0;

    /**
     * @brief Remove a previously registered callback.
     * @param callbackId The ID of the callback to remove
     */
    virtual void removeValueChangedCallback(std::uint32_t callbackId) = 0;
};

} // namespace nap

#endif // NAP_IPARAMETER_H
