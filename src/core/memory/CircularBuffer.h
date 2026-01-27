#ifndef NAP_CIRCULARBUFFER_H
#define NAP_CIRCULARBUFFER_H

#include <cstdint>
#include <memory>

namespace nap {

/**
 * @brief Lock-free circular buffer for audio sample storage.
 *
 * CircularBuffer provides a fixed-size, lock-free ring buffer optimized
 * for single-producer, single-consumer audio streaming scenarios.
 */
class CircularBuffer {
public:
    /**
     * @brief Construct a circular buffer with the specified capacity.
     * @param capacity Maximum number of samples the buffer can hold
     */
    explicit CircularBuffer(std::size_t capacity);
    ~CircularBuffer();

    CircularBuffer(const CircularBuffer&) = delete;
    CircularBuffer& operator=(const CircularBuffer&) = delete;
    CircularBuffer(CircularBuffer&&) noexcept;
    CircularBuffer& operator=(CircularBuffer&&) noexcept;

    /**
     * @brief Write samples to the buffer.
     * @param data Pointer to samples to write
     * @param numSamples Number of samples to write
     * @return Number of samples actually written
     */
    std::size_t write(const float* data, std::size_t numSamples);

    /**
     * @brief Read samples from the buffer.
     * @param data Pointer to destination for samples
     * @param numSamples Number of samples to read
     * @return Number of samples actually read
     */
    std::size_t read(float* data, std::size_t numSamples);

    /**
     * @brief Peek at samples without removing them from the buffer.
     * @param data Pointer to destination for samples
     * @param numSamples Number of samples to peek
     * @return Number of samples actually peeked
     */
    std::size_t peek(float* data, std::size_t numSamples) const;

    /**
     * @brief Skip samples without reading them.
     * @param numSamples Number of samples to skip
     * @return Number of samples actually skipped
     */
    std::size_t skip(std::size_t numSamples);

    /**
     * @brief Get the number of samples available for reading.
     * @return Number of available samples
     */
    std::size_t getAvailableForRead() const;

    /**
     * @brief Get the space available for writing.
     * @return Number of samples that can be written
     */
    std::size_t getAvailableForWrite() const;

    /**
     * @brief Get the total capacity of the buffer.
     * @return Buffer capacity in samples
     */
    std::size_t getCapacity() const;

    /**
     * @brief Check if the buffer is empty.
     * @return True if empty
     */
    bool isEmpty() const;

    /**
     * @brief Check if the buffer is full.
     * @return True if full
     */
    bool isFull() const;

    /**
     * @brief Clear all data from the buffer.
     */
    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_CIRCULARBUFFER_H
