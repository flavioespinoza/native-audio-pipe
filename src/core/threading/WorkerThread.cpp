#include "WorkerThread.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace nap {

class WorkerThread::Impl {
public:
    explicit Impl(const std::string& name, Priority priority)
        : name(name)
        , priority(priority)
        , running(false)
        , shouldStop(false)
        , taskPending(false)
    {
    }

    std::string name;
    Priority priority;
    std::atomic<bool> running;
    std::atomic<bool> shouldStop;
    std::atomic<bool> taskPending;
    TaskFunction task;
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable completionCv;
};

WorkerThread::WorkerThread(const std::string& name, Priority priority)
    : m_impl(std::make_unique<Impl>(name, priority))
{
}

WorkerThread::~WorkerThread()
{
    stop(true);
}

WorkerThread::WorkerThread(WorkerThread&&) noexcept = default;
WorkerThread& WorkerThread::operator=(WorkerThread&&) noexcept = default;

bool WorkerThread::start()
{
    if (m_impl->running) {
        return false;
    }

    m_impl->shouldStop = false;
    m_impl->running = true;

    m_impl->thread = std::thread([this]() {
        while (!m_impl->shouldStop) {
            std::unique_lock<std::mutex> lock(m_impl->mutex);
            m_impl->cv.wait(lock, [this]() {
                return m_impl->taskPending || m_impl->shouldStop;
            });

            if (m_impl->shouldStop) {
                break;
            }

            if (m_impl->task) {
                m_impl->task();
            }

            m_impl->taskPending = false;
            m_impl->completionCv.notify_all();
        }

        m_impl->running = false;
    });

    return true;
}

void WorkerThread::stop(bool waitForCompletion)
{
    m_impl->shouldStop = true;

    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->cv.notify_all();
    }

    if (waitForCompletion && m_impl->thread.joinable()) {
        m_impl->thread.join();
    }
}

bool WorkerThread::isRunning() const
{
    return m_impl->running;
}

void WorkerThread::setTask(TaskFunction task)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->task = std::move(task);
}

void WorkerThread::wake()
{
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->taskPending = true;
    }
    m_impl->cv.notify_one();
}

bool WorkerThread::waitForCompletion(std::uint32_t timeoutMs)
{
    std::unique_lock<std::mutex> lock(m_impl->mutex);
    return m_impl->completionCv.wait_for(lock,
        std::chrono::milliseconds(timeoutMs),
        [this]() { return !m_impl->taskPending; });
}

bool WorkerThread::setPriority(Priority priority)
{
    m_impl->priority = priority;
    // TODO: Implement platform-specific priority setting
    return true;
}

WorkerThread::Priority WorkerThread::getPriority() const
{
    return m_impl->priority;
}

std::string WorkerThread::getName() const
{
    return m_impl->name;
}

bool WorkerThread::setAffinity(std::uint64_t mask)
{
    // TODO: Implement platform-specific CPU affinity setting
    (void)mask;
    return true;
}

} // namespace nap
