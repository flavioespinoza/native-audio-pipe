#include "SpinLock.h"

namespace nap {

SpinLock::SpinLock()
    : m_flag(ATOMIC_FLAG_INIT)
{
}

SpinLock::~SpinLock() = default;

void SpinLock::lock()
{
    while (m_flag.test_and_set(std::memory_order_acquire)) {
        // Spin
        // On x86, we could add a pause instruction here for efficiency
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
        __builtin_ia32_pause();
#endif
    }
}

bool SpinLock::tryLock()
{
    return !m_flag.test_and_set(std::memory_order_acquire);
}

void SpinLock::unlock()
{
    m_flag.clear(std::memory_order_release);
}

bool SpinLock::isLocked() const
{
    // Note: This is a snapshot and may be stale
    return m_flag.test(std::memory_order_relaxed);
}

// SpinLockGuard implementation

SpinLockGuard::SpinLockGuard(SpinLock& lock)
    : m_lock(lock)
{
    m_lock.lock();
}

SpinLockGuard::~SpinLockGuard()
{
    m_lock.unlock();
}

// TrySpinLockGuard implementation

TrySpinLockGuard::TrySpinLockGuard(SpinLock& lock)
    : m_lock(lock)
    , m_ownsLock(lock.tryLock())
{
}

TrySpinLockGuard::~TrySpinLockGuard()
{
    if (m_ownsLock) {
        m_lock.unlock();
    }
}

bool TrySpinLockGuard::ownsLock() const
{
    return m_ownsLock;
}

TrySpinLockGuard::operator bool() const
{
    return m_ownsLock;
}

} // namespace nap
