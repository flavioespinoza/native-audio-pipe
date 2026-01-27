#include "ThreadBarrier.h"
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace nap {

class ThreadBarrier::Impl {
public:
    explicit Impl(std::uint32_t numThreads, CompletionFunction completionFunc)
        : numThreads(numThreads)
        , waitingCount(0)
        , generation(0)
        , completionFunc(std::move(completionFunc))
    {
    }

    std::uint32_t numThreads;
    std::atomic<std::uint32_t> waitingCount;
    std::atomic<std::uint64_t> generation;
    CompletionFunction completionFunc;
    std::mutex mutex;
    std::condition_variable cv;
};

ThreadBarrier::ThreadBarrier(std::uint32_t numThreads, CompletionFunction completionFunc)
    : m_impl(std::make_unique<Impl>(numThreads, std::move(completionFunc)))
{
}

ThreadBarrier::~ThreadBarrier() = default;

ThreadBarrier::ThreadBarrier(ThreadBarrier&&) noexcept = default;
ThreadBarrier& ThreadBarrier::operator=(ThreadBarrier&&) noexcept = default;

void ThreadBarrier::wait()
{
    std::unique_lock<std::mutex> lock(m_impl->mutex);

    std::uint64_t currentGen = m_impl->generation.load();
    std::uint32_t count = ++m_impl->waitingCount;

    if (count == m_impl->numThreads) {
        // Last thread to arrive
        m_impl->waitingCount = 0;
        ++m_impl->generation;

        if (m_impl->completionFunc) {
            m_impl->completionFunc();
        }

        m_impl->cv.notify_all();
    } else {
        // Wait for other threads
        m_impl->cv.wait(lock, [this, currentGen]() {
            return m_impl->generation.load() != currentGen;
        });
    }
}

bool ThreadBarrier::tryWait()
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->waitingCount.load() >= m_impl->numThreads - 1;
}

bool ThreadBarrier::waitFor(std::uint32_t timeoutMs)
{
    std::unique_lock<std::mutex> lock(m_impl->mutex);

    std::uint64_t currentGen = m_impl->generation.load();
    std::uint32_t count = ++m_impl->waitingCount;

    if (count == m_impl->numThreads) {
        m_impl->waitingCount = 0;
        ++m_impl->generation;

        if (m_impl->completionFunc) {
            m_impl->completionFunc();
        }

        m_impl->cv.notify_all();
        return true;
    }

    bool result = m_impl->cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
        [this, currentGen]() {
            return m_impl->generation.load() != currentGen;
        });

    if (!result) {
        --m_impl->waitingCount;
    }

    return result;
}

std::uint32_t ThreadBarrier::getNumThreads() const
{
    return m_impl->numThreads;
}

std::uint32_t ThreadBarrier::getWaitingCount() const
{
    return m_impl->waitingCount.load();
}

void ThreadBarrier::reset()
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->waitingCount = 0;
    ++m_impl->generation;
    m_impl->cv.notify_all();
}

void ThreadBarrier::reset(std::uint32_t numThreads)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->numThreads = numThreads;
    m_impl->waitingCount = 0;
    ++m_impl->generation;
    m_impl->cv.notify_all();
}

void ThreadBarrier::setCompletionFunction(CompletionFunction func)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->completionFunc = std::move(func);
}

std::uint64_t ThreadBarrier::getGeneration() const
{
    return m_impl->generation.load();
}

} // namespace nap
