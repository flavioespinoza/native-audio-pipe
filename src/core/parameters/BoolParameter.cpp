#include "core/parameters/BoolParameter.h"

namespace nap {

class BoolParameter::Impl {
public:
    std::string name;
    bool value;
    bool defaultValue;
    ChangeCallback changeCallback;

    Impl(const std::string& n, bool def)
        : name(n)
        , value(def)
        , defaultValue(def) {}
};

BoolParameter::BoolParameter(const std::string& name, bool defaultValue)
    : pImpl(std::make_unique<Impl>(name, defaultValue)) {}

BoolParameter::~BoolParameter() = default;

const std::string& BoolParameter::getName() const {
    return pImpl->name;
}

ParameterType BoolParameter::getType() const {
    return ParameterType::Bool;
}

void BoolParameter::reset() {
    setValue(pImpl->defaultValue);
}

bool BoolParameter::getValue() const {
    return pImpl->value;
}

void BoolParameter::setValue(bool value) {
    bool oldValue = pImpl->value;

    if (oldValue != value) {
        pImpl->value = value;
        if (pImpl->changeCallback) {
            pImpl->changeCallback(oldValue, value);
        }
    }
}

void BoolParameter::toggle() {
    setValue(!pImpl->value);
}

bool BoolParameter::isOn() const {
    return pImpl->value;
}

bool BoolParameter::isOff() const {
    return !pImpl->value;
}

bool BoolParameter::getDefaultValue() const {
    return pImpl->defaultValue;
}

void BoolParameter::setChangeCallback(ChangeCallback callback) {
    pImpl->changeCallback = std::move(callback);
}

void BoolParameter::removeChangeCallback() {
    pImpl->changeCallback = nullptr;
}

} // namespace nap
