#ifndef NAP_ENUM_PARAMETER_H
#define NAP_ENUM_PARAMETER_H

#include "api/IParameter.h"
#include <string>
#include <vector>
#include <functional>

namespace nap {

/**
 * @brief Enumeration parameter for dropdown-style selection
 *
 * Handles selection from a predefined list of string options
 * with change notification callbacks.
 */
class EnumParameter : public IParameter {
public:
    using ChangeCallback = std::function<void(size_t oldIndex, size_t newIndex)>;

    EnumParameter(const std::string& name,
                  const std::vector<std::string>& options,
                  size_t defaultIndex = 0);
    ~EnumParameter() override;

    // IParameter interface
    const std::string& getName() const override;
    ParameterType getType() const override;
    void reset() override;

    // Index-based access
    size_t getSelectedIndex() const;
    void setSelectedIndex(size_t index);

    // String-based access
    const std::string& getSelectedValue() const;
    bool setSelectedValue(const std::string& value);

    // Options management
    const std::vector<std::string>& getOptions() const;
    size_t getOptionCount() const;
    void setOptions(const std::vector<std::string>& options);
    void addOption(const std::string& option);

    // Navigation
    void selectNext();
    void selectPrevious();

    // Default
    size_t getDefaultIndex() const;

    // Callbacks
    void setChangeCallback(ChangeCallback callback);
    void removeChangeCallback();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_ENUM_PARAMETER_H
