#ifndef NAP_STATE_VECTOR_H
#define NAP_STATE_VECTOR_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace nap {

/**
 * @brief Lightweight container for snapshotting parameter data
 *
 * Provides efficient storage and comparison of parameter states
 * for undo/redo, automation, and interpolation.
 */
class StateVector {
public:
    StateVector();
    explicit StateVector(size_t capacity);
    ~StateVector();

    // Copy and move
    StateVector(const StateVector& other);
    StateVector& operator=(const StateVector& other);
    StateVector(StateVector&&) noexcept;
    StateVector& operator=(StateVector&&) noexcept;

    // Capacity
    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    void reserve(size_t newCapacity);
    void resize(size_t newSize);
    void clear();

    // Float values
    void setFloat(size_t index, float value);
    float getFloat(size_t index) const;
    void pushFloat(float value);

    // Int values
    void setInt(size_t index, int32_t value);
    int32_t getInt(size_t index) const;
    void pushInt(int32_t value);

    // Raw access
    const float* data() const;
    float* data();

    // Comparison
    bool equals(const StateVector& other, float tolerance = 1e-6f) const;
    float distance(const StateVector& other) const;

    // Interpolation
    static StateVector lerp(const StateVector& a, const StateVector& b, float t);

    // Serialization helpers
    std::vector<uint8_t> toBytes() const;
    bool fromBytes(const std::vector<uint8_t>& bytes);

    // Operators
    float& operator[](size_t index);
    const float& operator[](size_t index) const;
    bool operator==(const StateVector& other) const;
    bool operator!=(const StateVector& other) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_STATE_VECTOR_H
