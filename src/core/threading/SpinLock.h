#ifndef NAP_SPINLOCK_H
#define NAP_SPINLOCK_H

#include <atomic>
#include <cstdint>

namespace nap {

/**
 * @brief Lightweight spin lock for real-time audio processing.
 *
 * SpinLock provides a minimal-overhead synchronization primitive suitable
 * for protecting very short critical sections in real-time audio code.
 */
class SpinLock {
public:
    SpinLock();
    ~SpinLock();

    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;

    /**
     * @brief Acquire the lock, spinning until successful.
     */
    void lock();

    /**
     * @brief Try to acquire the lock without blocking.
     * @return True if the lock was acquired
     */
    bool tryLock();

    /**
     * @brief Release the lock.
     */
    void unlock();

    /**
     * @brief Check if the lock is currently held.
     * @return True if locked
     */
    bool isLocked() const;

private:
    std::atomic_flag m_flag;
};

/**
 * @brief RAII guard for SpinLock.
 */
class SpinLockGuard {
public:
    /**
     * @brief Construct a guard that acquires the lock.
     * @param lock The SpinLock to guard
     */
    explicit SpinLockGuard(SpinLock& lock);

    /**
     * @brief Destroy the guard, releasing the lock.
     */
    ~SpinLockGuard();

    SpinLockGuard(const SpinLockGuard&) = delete;
    SpinLockGuard& operator=(const SpinLockGuard&) = delete;

private:
    SpinLock& m_lock;
};

/**
 * @brief Try-lock RAII guard for SpinLock.
 */
class TrySpinLockGuard {
public:
    /**
     * @brief Construct a guard that tries to acquire the lock.
     * @param lock The SpinLock to guard
     */
    explicit TrySpinLockGuard(SpinLock& lock);

    /**
     * @brief Destroy the guard, releasing the lock if owned.
     */
    ~TrySpinLockGuard();

    TrySpinLockGuard(const TrySpinLockGuard&) = delete;
    TrySpinLockGuard& operator=(const TrySpinLockGuard&) = delete;

    /**
     * @brief Check if the lock was successfully acquired.
     * @return True if the lock is owned
     */
    bool ownsLock() const;

    explicit operator bool() const;

private:
    SpinLock& m_lock;
    bool m_ownsLock;
};

} // namespace nap

#endif // NAP_SPINLOCK_H
