#ifndef NAP_BOOL_PARAMETER_H
#define NAP_BOOL_PARAMETER_H

#include "api/IParameter.h"
#include <string>
#include <functional>

namespace nap {

/**
 * @brief Boolean parameter for toggle states
 *
 * Handles on/off toggle parameter values with change notification callbacks.
 */
class BoolParameter : public IParameter {
public:
    using ChangeCallback = std::function<void(bool oldValue, bool newValue)>;

    BoolParameter(const std::string& name, bool defaultValue = false);
    ~BoolParameter() override;

    // IParameter interface
    const std::string& getName() const override;
    ParameterType getType() const override;
    void reset() override;

    // Value access
    bool getValue() const;
    void setValue(bool value);

    // Convenience
    void toggle();
    bool isOn() const;
    bool isOff() const;

    // Default
    bool getDefaultValue() const;

    // Callbacks
    void setChangeCallback(ChangeCallback callback);
    void removeChangeCallback();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_BOOL_PARAMETER_H
