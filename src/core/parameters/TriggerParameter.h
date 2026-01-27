#ifndef NAP_TRIGGER_PARAMETER_H
#define NAP_TRIGGER_PARAMETER_H

#include "api/IParameter.h"
#include <string>
#include <functional>
#include <atomic>

namespace nap {

/**
 * @brief Trigger parameter for momentary events
 *
 * Handles one-shot trigger events (like a button press).
 * The trigger auto-resets after being consumed.
 */
class TriggerParameter : public IParameter {
public:
    using TriggerCallback = std::function<void()>;

    explicit TriggerParameter(const std::string& name);
    ~TriggerParameter() override;

    // IParameter interface
    const std::string& getName() const override;
    ParameterType getType() const override;
    void reset() override;

    // Trigger operations
    void trigger();
    bool consume();  // Returns true if triggered, then resets
    bool isPending() const;

    // Callbacks
    void setTriggerCallback(TriggerCallback callback);
    void removeTriggerCallback();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_TRIGGER_PARAMETER_H
