#include "core/serialization/StateVector.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace nap {

class StateVector::Impl {
public:
    std::vector<float> data;

    Impl() = default;
    explicit Impl(size_t capacity) { data.reserve(capacity); }
};

StateVector::StateVector()
    : pImpl(std::make_unique<Impl>()) {}

StateVector::StateVector(size_t capacity)
    : pImpl(std::make_unique<Impl>(capacity)) {}

StateVector::~StateVector() = default;

StateVector::StateVector(const StateVector& other)
    : pImpl(std::make_unique<Impl>()) {
    pImpl->data = other.pImpl->data;
}

StateVector& StateVector::operator=(const StateVector& other) {
    if (this != &other) {
        pImpl->data = other.pImpl->data;
    }
    return *this;
}

StateVector::StateVector(StateVector&&) noexcept = default;
StateVector& StateVector::operator=(StateVector&&) noexcept = default;

size_t StateVector::size() const {
    return pImpl->data.size();
}

size_t StateVector::capacity() const {
    return pImpl->data.capacity();
}

bool StateVector::empty() const {
    return pImpl->data.empty();
}

void StateVector::reserve(size_t newCapacity) {
    pImpl->data.reserve(newCapacity);
}

void StateVector::resize(size_t newSize) {
    pImpl->data.resize(newSize, 0.0f);
}

void StateVector::clear() {
    pImpl->data.clear();
}

void StateVector::setFloat(size_t index, float value) {
    if (index < pImpl->data.size()) {
        pImpl->data[index] = value;
    }
}

float StateVector::getFloat(size_t index) const {
    if (index < pImpl->data.size()) {
        return pImpl->data[index];
    }
    return 0.0f;
}

void StateVector::pushFloat(float value) {
    pImpl->data.push_back(value);
}

void StateVector::setInt(size_t index, int32_t value) {
    float floatVal;
    std::memcpy(&floatVal, &value, sizeof(float));
    setFloat(index, floatVal);
}

int32_t StateVector::getInt(size_t index) const {
    float floatVal = getFloat(index);
    int32_t intVal;
    std::memcpy(&intVal, &floatVal, sizeof(int32_t));
    return intVal;
}

void StateVector::pushInt(int32_t value) {
    float floatVal;
    std::memcpy(&floatVal, &value, sizeof(float));
    pushFloat(floatVal);
}

const float* StateVector::data() const {
    return pImpl->data.data();
}

float* StateVector::data() {
    return pImpl->data.data();
}

bool StateVector::equals(const StateVector& other, float tolerance) const {
    if (size() != other.size()) return false;

    for (size_t i = 0; i < size(); ++i) {
        if (std::abs(pImpl->data[i] - other.pImpl->data[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

float StateVector::distance(const StateVector& other) const {
    if (size() != other.size()) return std::numeric_limits<float>::max();

    float sumSq = 0.0f;
    for (size_t i = 0; i < size(); ++i) {
        float diff = pImpl->data[i] - other.pImpl->data[i];
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq);
}

StateVector StateVector::lerp(const StateVector& a, const StateVector& b, float t) {
    size_t size = std::min(a.size(), b.size());
    StateVector result(size);
    result.resize(size);

    t = std::clamp(t, 0.0f, 1.0f);
    for (size_t i = 0; i < size; ++i) {
        result.pImpl->data[i] = a.pImpl->data[i] + t * (b.pImpl->data[i] - a.pImpl->data[i]);
    }

    return result;
}

std::vector<uint8_t> StateVector::toBytes() const {
    std::vector<uint8_t> bytes;
    bytes.resize(sizeof(uint32_t) + pImpl->data.size() * sizeof(float));

    uint32_t count = static_cast<uint32_t>(pImpl->data.size());
    std::memcpy(bytes.data(), &count, sizeof(uint32_t));
    std::memcpy(bytes.data() + sizeof(uint32_t), pImpl->data.data(),
                pImpl->data.size() * sizeof(float));

    return bytes;
}

bool StateVector::fromBytes(const std::vector<uint8_t>& bytes) {
    if (bytes.size() < sizeof(uint32_t)) return false;

    uint32_t count;
    std::memcpy(&count, bytes.data(), sizeof(uint32_t));

    size_t expectedSize = sizeof(uint32_t) + count * sizeof(float);
    if (bytes.size() < expectedSize) return false;

    pImpl->data.resize(count);
    std::memcpy(pImpl->data.data(), bytes.data() + sizeof(uint32_t),
                count * sizeof(float));

    return true;
}

float& StateVector::operator[](size_t index) {
    return pImpl->data[index];
}

const float& StateVector::operator[](size_t index) const {
    return pImpl->data[index];
}

bool StateVector::operator==(const StateVector& other) const {
    return equals(other, 0.0f);
}

bool StateVector::operator!=(const StateVector& other) const {
    return !(*this == other);
}

} // namespace nap
