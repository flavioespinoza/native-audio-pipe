#ifndef NAP_TASKQUEUE_H
#define NAP_TASKQUEUE_H

#include <cstdint>
#include <functional>
#include <memory>

namespace nap {

/**
 * @brief Thread-safe task queue for asynchronous audio processing.
 *
 * TaskQueue provides a lock-free or low-contention queue for passing
 * tasks between threads in a real-time audio context.
 */
class TaskQueue {
public:
    using Task = std::function<void()>;

    /**
     * @brief Construct a task queue with the specified capacity.
     * @param capacity Maximum number of pending tasks
     */
    explicit TaskQueue(std::size_t capacity = 1024);
    ~TaskQueue();

    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    TaskQueue(TaskQueue&&) noexcept;
    TaskQueue& operator=(TaskQueue&&) noexcept;

    /**
     * @brief Push a task onto the queue.
     * @param task The task to push
     * @return True if the task was pushed successfully
     */
    bool push(Task task);

    /**
     * @brief Try to pop a task from the queue.
     * @param task Reference to store the popped task
     * @return True if a task was popped
     */
    bool tryPop(Task& task);

    /**
     * @brief Pop a task, blocking until one is available.
     * @param task Reference to store the popped task
     * @param timeoutMs Maximum time to wait in milliseconds
     * @return True if a task was popped within the timeout
     */
    bool pop(Task& task, std::uint32_t timeoutMs = 0);

    /**
     * @brief Execute all pending tasks.
     * @return Number of tasks executed
     */
    std::size_t executeAll();

    /**
     * @brief Execute up to N pending tasks.
     * @param maxTasks Maximum number of tasks to execute
     * @return Number of tasks executed
     */
    std::size_t executeN(std::size_t maxTasks);

    /**
     * @brief Get the number of pending tasks.
     * @return Task count
     */
    std::size_t size() const;

    /**
     * @brief Check if the queue is empty.
     * @return True if empty
     */
    bool isEmpty() const;

    /**
     * @brief Check if the queue is full.
     * @return True if full
     */
    bool isFull() const;

    /**
     * @brief Get the queue capacity.
     * @return Maximum number of tasks
     */
    std::size_t getCapacity() const;

    /**
     * @brief Clear all pending tasks.
     */
    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_TASKQUEUE_H
