#ifndef NAP_IPARAMETER_H
#define NAP_IPARAMETER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace nap {

/**
 * @brief Parameter type enumeration
 */
enum class ParameterType {
    Float,      ///< Continuous floating-point value
    Int,        ///< Discrete integer value
    Bool,       ///< Boolean toggle
    Enum,       ///< Selection from enumerated options
    Trigger,    ///< Momentary trigger/event
    String,     ///< Text value
    Custom      ///< User-defined type
};

/**
 * @brief Base interface for all parameter types.
 *
 * IParameter defines the minimal contract for parameters that can be
 * attached to audio nodes. Specialized parameter types (FloatParameter,
 * IntParameter, etc.) provide type-specific functionality.
 */
class IParameter {
public:
    virtual ~IParameter() = default;

    /**
     * @brief Get the parameter name.
     * @return Parameter name as a string
     */
    virtual const std::string& getName() const = 0;

    /**
     * @brief Get the parameter type.
     * @return ParameterType enum value
     */
    virtual ParameterType getType() const = 0;

    /**
     * @brief Reset the parameter to its default value.
     */
    virtual void reset() = 0;
};

/**
 * @brief Extended interface for numeric parameters with automation support.
 *
 * INumericParameter extends IParameter with value access, normalization,
 * range constraints, and smoothing for real-time modulation.
 */
class INumericParameter : public IParameter {
public:
    ~INumericParameter() override = default;

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
     * @brief Get the smoothed value for real-time processing.
     * @param sampleRate The current sample rate for smoothing calculation
     * @return Smoothed value
     */
    virtual float getSmoothedValue(float sampleRate) = 0;

    /**
     * @brief Enable or disable value smoothing.
     * @param enable True to enable smoothing
     * @param smoothingTime Smoothing time in seconds
     */
    virtual void enableSmoothing(bool enable, float smoothingTime = 0.01f) = 0;

    /**
     * @brief Get the unit label for display (e.g., "dB", "Hz", "%").
     * @return Unit label as a string
     */
    virtual std::string getUnitLabel() const { return ""; }

    using ValueChangedCallback = std::function<void(float oldValue, float newValue)>;

    /**
     * @brief Register a callback for value changes.
     * @param callback The callback function to register
     */
    virtual void setValueChangedCallback(ValueChangedCallback callback) = 0;
};

} // namespace nap

#endif // NAP_IPARAMETER_H
