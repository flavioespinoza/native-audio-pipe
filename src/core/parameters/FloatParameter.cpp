#include "core/parameters/FloatParameter.h"
#include <algorithm>
#include <cmath>

namespace nap {

class FloatParameter::Impl {
public:
    std::string name;
    float value;
    float defaultValue;
    float minValue;
    float maxValue;

    // Smoothing state
    bool smoothingEnabled = false;
    float smoothingTime = 0.01f;
    float smoothedValue;
    float smoothingCoeff = 0.0f;

    ChangeCallback changeCallback;

    Impl(const std::string& n, float def, float min, float max)
        : name(n)
        , value(std::clamp(def, min, max))
        , defaultValue(def)
        , minValue(min)
        , maxValue(max)
        , smoothedValue(value) {}

    float clamp(float v) const {
        return std::clamp(v, minValue, maxValue);
    }

    float normalize(float v) const {
        if (maxValue == minValue) return 0.0f;
        return (v - minValue) / (maxValue - minValue);
    }

    float denormalize(float normalized) const {
        return minValue + normalized * (maxValue - minValue);
    }
};

FloatParameter::FloatParameter(const std::string& name, float defaultValue,
                               float minValue, float maxValue)
    : pImpl(std::make_unique<Impl>(name, defaultValue, minValue, maxValue)) {}

FloatParameter::~FloatParameter() = default;

const std::string& FloatParameter::getName() const {
    return pImpl->name;
}

ParameterType FloatParameter::getType() const {
    return ParameterType::Float;
}

void FloatParameter::reset() {
    setValue(pImpl->defaultValue);
    pImpl->smoothedValue = pImpl->value;
}

float FloatParameter::getValue() const {
    return pImpl->value;
}

void FloatParameter::setValue(float value) {
    float oldValue = pImpl->value;
    float newValue = pImpl->clamp(value);

    if (oldValue != newValue) {
        pImpl->value = newValue;
        if (pImpl->changeCallback) {
            pImpl->changeCallback(oldValue, newValue);
        }
    }
}

float FloatParameter::getNormalizedValue() const {
    return pImpl->normalize(pImpl->value);
}

void FloatParameter::setNormalizedValue(float normalized) {
    setValue(pImpl->denormalize(std::clamp(normalized, 0.0f, 1.0f)));
}

float FloatParameter::getMinValue() const {
    return pImpl->minValue;
}

float FloatParameter::getMaxValue() const {
    return pImpl->maxValue;
}

float FloatParameter::getDefaultValue() const {
    return pImpl->defaultValue;
}

void FloatParameter::setRange(float minValue, float maxValue) {
    pImpl->minValue = minValue;
    pImpl->maxValue = maxValue;
    pImpl->value = pImpl->clamp(pImpl->value);
}

void FloatParameter::enableSmoothing(bool enable, float smoothingTime) {
    pImpl->smoothingEnabled = enable;
    pImpl->smoothingTime = smoothingTime;
    if (!enable) {
        pImpl->smoothedValue = pImpl->value;
    }
}

bool FloatParameter::isSmoothingEnabled() const {
    return pImpl->smoothingEnabled;
}

float FloatParameter::getSmoothedValue(float sampleRate) {
    if (!pImpl->smoothingEnabled) {
        return pImpl->value;
    }

    // One-pole smoothing filter
    float coeff = std::exp(-1.0f / (pImpl->smoothingTime * sampleRate));
    pImpl->smoothedValue = pImpl->value + coeff * (pImpl->smoothedValue - pImpl->value);
    return pImpl->smoothedValue;
}

void FloatParameter::setChangeCallback(ChangeCallback callback) {
    pImpl->changeCallback = std::move(callback);
}

void FloatParameter::removeChangeCallback() {
    pImpl->changeCallback = nullptr;
}

} // namespace nap
