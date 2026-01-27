#include "CircularBuffer.h"
#include <algorithm>
#include <atomic>
#include <vector>

namespace nap {

class CircularBuffer::Impl {
public:
    explicit Impl(std::size_t capacity)
        : buffer(capacity, 0.0f)
        , capacity(capacity)
        , readIndex(0)
        , writeIndex(0)
    {
    }

    std::vector<float> buffer;
    std::size_t capacity;
    std::atomic<std::size_t> readIndex;
    std::atomic<std::size_t> writeIndex;
};

CircularBuffer::CircularBuffer(std::size_t capacity)
    : m_impl(std::make_unique<Impl>(capacity))
{
}

CircularBuffer::~CircularBuffer() = default;

CircularBuffer::CircularBuffer(CircularBuffer&&) noexcept = default;
CircularBuffer& CircularBuffer::operator=(CircularBuffer&&) noexcept = default;

std::size_t CircularBuffer::write(const float* data, std::size_t numSamples)
{
    const std::size_t available = getAvailableForWrite();
    const std::size_t toWrite = std::min(numSamples, available);

    std::size_t writeIdx = m_impl->writeIndex.load(std::memory_order_relaxed);

    for (std::size_t i = 0; i < toWrite; ++i) {
        m_impl->buffer[writeIdx] = data[i];
        writeIdx = (writeIdx + 1) % m_impl->capacity;
    }

    m_impl->writeIndex.store(writeIdx, std::memory_order_release);
    return toWrite;
}

std::size_t CircularBuffer::read(float* data, std::size_t numSamples)
{
    const std::size_t available = getAvailableForRead();
    const std::size_t toRead = std::min(numSamples, available);

    std::size_t readIdx = m_impl->readIndex.load(std::memory_order_relaxed);

    for (std::size_t i = 0; i < toRead; ++i) {
        data[i] = m_impl->buffer[readIdx];
        readIdx = (readIdx + 1) % m_impl->capacity;
    }

    m_impl->readIndex.store(readIdx, std::memory_order_release);
    return toRead;
}

std::size_t CircularBuffer::peek(float* data, std::size_t numSamples) const
{
    const std::size_t available = getAvailableForRead();
    const std::size_t toPeek = std::min(numSamples, available);

    std::size_t readIdx = m_impl->readIndex.load(std::memory_order_acquire);

    for (std::size_t i = 0; i < toPeek; ++i) {
        data[i] = m_impl->buffer[readIdx];
        readIdx = (readIdx + 1) % m_impl->capacity;
    }

    return toPeek;
}

std::size_t CircularBuffer::skip(std::size_t numSamples)
{
    const std::size_t available = getAvailableForRead();
    const std::size_t toSkip = std::min(numSamples, available);

    std::size_t readIdx = m_impl->readIndex.load(std::memory_order_relaxed);
    readIdx = (readIdx + toSkip) % m_impl->capacity;
    m_impl->readIndex.store(readIdx, std::memory_order_release);

    return toSkip;
}

std::size_t CircularBuffer::getAvailableForRead() const
{
    const std::size_t writeIdx = m_impl->writeIndex.load(std::memory_order_acquire);
    const std::size_t readIdx = m_impl->readIndex.load(std::memory_order_acquire);

    if (writeIdx >= readIdx) {
        return writeIdx - readIdx;
    }
    return m_impl->capacity - readIdx + writeIdx;
}

std::size_t CircularBuffer::getAvailableForWrite() const
{
    return m_impl->capacity - getAvailableForRead() - 1;
}

std::size_t CircularBuffer::getCapacity() const
{
    return m_impl->capacity;
}

bool CircularBuffer::isEmpty() const
{
    return getAvailableForRead() == 0;
}

bool CircularBuffer::isFull() const
{
    return getAvailableForWrite() == 0;
}

void CircularBuffer::clear()
{
    m_impl->readIndex.store(0, std::memory_order_release);
    m_impl->writeIndex.store(0, std::memory_order_release);
}

} // namespace nap
