#include "core/parameters/TriggerParameter.h"

namespace nap {

class TriggerParameter::Impl {
public:
    std::string name;
    std::atomic<bool> triggered{false};
    TriggerCallback triggerCallback;

    explicit Impl(const std::string& n) : name(n) {}
};

TriggerParameter::TriggerParameter(const std::string& name)
    : pImpl(std::make_unique<Impl>(name)) {}

TriggerParameter::~TriggerParameter() = default;

const std::string& TriggerParameter::getName() const {
    return pImpl->name;
}

ParameterType TriggerParameter::getType() const {
    return ParameterType::Trigger;
}

void TriggerParameter::reset() {
    pImpl->triggered.store(false, std::memory_order_release);
}

void TriggerParameter::trigger() {
    pImpl->triggered.store(true, std::memory_order_release);
    if (pImpl->triggerCallback) {
        pImpl->triggerCallback();
    }
}

bool TriggerParameter::consume() {
    return pImpl->triggered.exchange(false, std::memory_order_acq_rel);
}

bool TriggerParameter::isPending() const {
    return pImpl->triggered.load(std::memory_order_acquire);
}

void TriggerParameter::setTriggerCallback(TriggerCallback callback) {
    pImpl->triggerCallback = std::move(callback);
}

void TriggerParameter::removeTriggerCallback() {
    pImpl->triggerCallback = nullptr;
}

} // namespace nap
