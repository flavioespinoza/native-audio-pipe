#ifndef NAP_THREADBARRIER_H
#define NAP_THREADBARRIER_H

#include <cstdint>
#include <functional>
#include <memory>

namespace nap {

/**
 * @brief Thread synchronization barrier for parallel audio processing.
 *
 * ThreadBarrier allows multiple threads to synchronize at a common point,
 * useful for parallel audio processing phases.
 */
class ThreadBarrier {
public:
    using CompletionFunction = std::function<void()>;

    /**
     * @brief Construct a barrier for the specified number of threads.
     * @param numThreads Number of threads that must arrive
     * @param completionFunc Optional function called when all threads arrive
     */
    explicit ThreadBarrier(std::uint32_t numThreads,
                           CompletionFunction completionFunc = nullptr);
    ~ThreadBarrier();

    ThreadBarrier(const ThreadBarrier&) = delete;
    ThreadBarrier& operator=(const ThreadBarrier&) = delete;
    ThreadBarrier(ThreadBarrier&&) noexcept;
    ThreadBarrier& operator=(ThreadBarrier&&) noexcept;

    /**
     * @brief Wait at the barrier until all threads arrive.
     */
    void wait();

    /**
     * @brief Try to pass the barrier without blocking.
     * @return True if all threads have arrived
     */
    bool tryWait();

    /**
     * @brief Wait at the barrier with a timeout.
     * @param timeoutMs Maximum time to wait in milliseconds
     * @return True if all threads arrived within the timeout
     */
    bool waitFor(std::uint32_t timeoutMs);

    /**
     * @brief Get the number of threads this barrier is configured for.
     * @return Thread count
     */
    std::uint32_t getNumThreads() const;

    /**
     * @brief Get the number of threads currently waiting.
     * @return Waiting thread count
     */
    std::uint32_t getWaitingCount() const;

    /**
     * @brief Reset the barrier for reuse.
     */
    void reset();

    /**
     * @brief Reset the barrier with a new thread count.
     * @param numThreads New number of threads
     */
    void reset(std::uint32_t numThreads);

    /**
     * @brief Set the completion function.
     * @param func The function to call when all threads arrive
     */
    void setCompletionFunction(CompletionFunction func);

    /**
     * @brief Get the current generation/phase of the barrier.
     * @return Generation counter
     */
    std::uint64_t getGeneration() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_THREADBARRIER_H
