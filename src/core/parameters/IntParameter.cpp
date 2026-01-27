#include "core/parameters/IntParameter.h"
#include <algorithm>
#include <cmath>

namespace nap {

class IntParameter::Impl {
public:
    std::string name;
    int value;
    int defaultValue;
    int minValue;
    int maxValue;
    int stepSize;
    ChangeCallback changeCallback;

    Impl(const std::string& n, int def, int min, int max, int step)
        : name(n)
        , defaultValue(def)
        , minValue(min)
        , maxValue(max)
        , stepSize(step) {
        value = clamp(quantize(def));
    }

    int clamp(int v) const {
        return std::clamp(v, minValue, maxValue);
    }

    int quantize(int v) const {
        if (stepSize <= 1) return v;
        int steps = (v - minValue) / stepSize;
        return minValue + steps * stepSize;
    }
};

IntParameter::IntParameter(const std::string& name, int defaultValue,
                           int minValue, int maxValue, int stepSize)
    : pImpl(std::make_unique<Impl>(name, defaultValue, minValue, maxValue, stepSize)) {}

IntParameter::~IntParameter() = default;

const std::string& IntParameter::getName() const {
    return pImpl->name;
}

ParameterType IntParameter::getType() const {
    return ParameterType::Int;
}

void IntParameter::reset() {
    setValue(pImpl->defaultValue);
}

int IntParameter::getValue() const {
    return pImpl->value;
}

void IntParameter::setValue(int value) {
    int oldValue = pImpl->value;
    int newValue = pImpl->clamp(pImpl->quantize(value));

    if (oldValue != newValue) {
        pImpl->value = newValue;
        if (pImpl->changeCallback) {
            pImpl->changeCallback(oldValue, newValue);
        }
    }
}

float IntParameter::getNormalizedValue() const {
    if (pImpl->maxValue == pImpl->minValue) return 0.0f;
    return static_cast<float>(pImpl->value - pImpl->minValue) /
           static_cast<float>(pImpl->maxValue - pImpl->minValue);
}

void IntParameter::setNormalizedValue(float normalized) {
    normalized = std::clamp(normalized, 0.0f, 1.0f);
    int value = pImpl->minValue +
        static_cast<int>(std::round(normalized * (pImpl->maxValue - pImpl->minValue)));
    setValue(value);
}

int IntParameter::getMinValue() const {
    return pImpl->minValue;
}

int IntParameter::getMaxValue() const {
    return pImpl->maxValue;
}

int IntParameter::getDefaultValue() const {
    return pImpl->defaultValue;
}

int IntParameter::getStepSize() const {
    return pImpl->stepSize;
}

void IntParameter::setRange(int minValue, int maxValue) {
    pImpl->minValue = minValue;
    pImpl->maxValue = maxValue;
    pImpl->value = pImpl->clamp(pImpl->value);
}

void IntParameter::setStepSize(int stepSize) {
    pImpl->stepSize = std::max(1, stepSize);
    pImpl->value = pImpl->quantize(pImpl->value);
}

void IntParameter::increment() {
    setValue(pImpl->value + pImpl->stepSize);
}

void IntParameter::decrement() {
    setValue(pImpl->value - pImpl->stepSize);
}

void IntParameter::setChangeCallback(ChangeCallback callback) {
    pImpl->changeCallback = std::move(callback);
}

void IntParameter::removeChangeCallback() {
    pImpl->changeCallback = nullptr;
}

} // namespace nap
