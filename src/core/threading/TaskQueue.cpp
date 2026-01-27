#include "TaskQueue.h"
#include <condition_variable>
#include <mutex>
#include <queue>

namespace nap {

class TaskQueue::Impl {
public:
    explicit Impl(std::size_t capacity)
        : capacity(capacity)
    {
    }

    std::queue<Task> tasks;
    std::mutex mutex;
    std::condition_variable cv;
    std::size_t capacity;
};

TaskQueue::TaskQueue(std::size_t capacity)
    : m_impl(std::make_unique<Impl>(capacity))
{
}

TaskQueue::~TaskQueue() = default;

TaskQueue::TaskQueue(TaskQueue&&) noexcept = default;
TaskQueue& TaskQueue::operator=(TaskQueue&&) noexcept = default;

bool TaskQueue::push(Task task)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);

    if (m_impl->tasks.size() >= m_impl->capacity) {
        return false;
    }

    m_impl->tasks.push(std::move(task));
    m_impl->cv.notify_one();
    return true;
}

bool TaskQueue::tryPop(Task& task)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);

    if (m_impl->tasks.empty()) {
        return false;
    }

    task = std::move(m_impl->tasks.front());
    m_impl->tasks.pop();
    return true;
}

bool TaskQueue::pop(Task& task, std::uint32_t timeoutMs)
{
    std::unique_lock<std::mutex> lock(m_impl->mutex);

    if (timeoutMs > 0) {
        if (!m_impl->cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                                  [this]() { return !m_impl->tasks.empty(); })) {
            return false;
        }
    } else {
        m_impl->cv.wait(lock, [this]() { return !m_impl->tasks.empty(); });
    }

    task = std::move(m_impl->tasks.front());
    m_impl->tasks.pop();
    return true;
}

std::size_t TaskQueue::executeAll()
{
    std::size_t count = 0;
    Task task;

    while (tryPop(task)) {
        task();
        ++count;
    }

    return count;
}

std::size_t TaskQueue::executeN(std::size_t maxTasks)
{
    std::size_t count = 0;
    Task task;

    while (count < maxTasks && tryPop(task)) {
        task();
        ++count;
    }

    return count;
}

std::size_t TaskQueue::size() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->tasks.size();
}

bool TaskQueue::isEmpty() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->tasks.empty();
}

bool TaskQueue::isFull() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->tasks.size() >= m_impl->capacity;
}

std::size_t TaskQueue::getCapacity() const
{
    return m_impl->capacity;
}

void TaskQueue::clear()
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    while (!m_impl->tasks.empty()) {
        m_impl->tasks.pop();
    }
}

} // namespace nap
