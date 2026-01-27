#ifndef NAP_WORKERTHREAD_H
#define NAP_WORKERTHREAD_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace nap {

/**
 * @brief Managed worker thread for audio processing tasks.
 *
 * WorkerThread provides a controlled thread abstraction with priority
 * settings and lifecycle management suitable for real-time audio processing.
 */
class WorkerThread {
public:
    enum class Priority {
        Low,
        Normal,
        High,
        Realtime
    };

    using TaskFunction = std::function<void()>;

    /**
     * @brief Construct a worker thread with the specified name.
     * @param name Thread name for debugging
     * @param priority Thread priority level
     */
    explicit WorkerThread(const std::string& name = "WorkerThread",
                          Priority priority = Priority::Normal);
    ~WorkerThread();

    WorkerThread(const WorkerThread&) = delete;
    WorkerThread& operator=(const WorkerThread&) = delete;
    WorkerThread(WorkerThread&&) noexcept;
    WorkerThread& operator=(WorkerThread&&) noexcept;

    /**
     * @brief Start the worker thread.
     * @return True if started successfully
     */
    bool start();

    /**
     * @brief Stop the worker thread.
     * @param waitForCompletion If true, wait for current task to complete
     */
    void stop(bool waitForCompletion = true);

    /**
     * @brief Check if the thread is running.
     * @return True if running
     */
    bool isRunning() const;

    /**
     * @brief Set the main task to execute repeatedly.
     * @param task The task function
     */
    void setTask(TaskFunction task);

    /**
     * @brief Wake the thread to execute its task.
     */
    void wake();

    /**
     * @brief Wait for the thread to complete one iteration.
     * @param timeoutMs Maximum time to wait in milliseconds
     * @return True if completed within timeout
     */
    bool waitForCompletion(std::uint32_t timeoutMs = 1000);

    /**
     * @brief Set the thread priority.
     * @param priority The priority level
     * @return True if priority was set successfully
     */
    bool setPriority(Priority priority);

    /**
     * @brief Get the thread priority.
     * @return Current priority level
     */
    Priority getPriority() const;

    /**
     * @brief Get the thread name.
     * @return Thread name
     */
    std::string getName() const;

    /**
     * @brief Set the CPU affinity mask.
     * @param mask CPU affinity mask
     * @return True if affinity was set successfully
     */
    bool setAffinity(std::uint64_t mask);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_WORKERTHREAD_H
