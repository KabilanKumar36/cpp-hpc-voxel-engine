#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace Core {

/**
 * @class ThreadSafeQueue
 * @brief A thread-safe FIFO queue wrapper using mutexes and condition variables.
 * Designed for producer-consumer scenarios (like ThreadPools).
 */
template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    /**
     * @brief Pushes a value into the queue (Move semantics).
     */
    void push(T&& objValue) {
        {
            std::scoped_lock objLock(m_objMutex);
            m_queJobs.push(std::move(objValue));
        }
        m_objContVar.notify_one();
    }

    /**
     * @brief Pushes a value into the queue (Copy semantics).
     */
    void push(const T& objValue) {
        {
            std::scoped_lock objLock(m_objMutex);
            m_queJobs.push(objValue);
        }
        m_objContVar.notify_one();
    }

    /**
     * @brief Waits until the queue is not empty, then pops the front element.
     * @param objValue Reference to store the popped value.
     * @return True if value popped, False if queue was invalidated/stopped.
     */
    bool wait_and_pop(T& objValue) {
        std::unique_lock<std::mutex> objLock(m_objMutex);

        // Wait until queue has items OR queue is invalidated (stopped)
        m_objContVar.wait(objLock, [this] { return !m_queJobs.empty() || m_bInvalidated; });

        if (m_bInvalidated && m_queJobs.empty()) {
            return false;
        }

        objValue = std::move(m_queJobs.front());
        m_queJobs.pop();

        return true;
    }

    /**
     * @brief Non-blocking attempt to pop an item.
     * @return std::optional containing the item if found, nullopt otherwise.
     */
    std::optional<T> try_pop() {
        std::scoped_lock objLock(m_objMutex);
        if (m_queJobs.empty()) {
            return std::nullopt;
        }
        T objValue = std::move(m_queJobs.front());
        m_queJobs.pop();
        return objValue;
    }

    bool empty() const {
        std::scoped_lock objLock(m_objMutex);
        return m_queJobs.empty();
    }

    /**
     * @brief Wakes up all waiting threads effectively cancelling the wait.
     */
    void invalidate() {
        {
            std::scoped_lock objLock(m_objMutex);
            m_bInvalidated = true;
        }
        m_objContVar.notify_all();
    }

private:
    std::queue<T> m_queJobs;
    mutable std::mutex m_objMutex;
    std::condition_variable m_objContVar;
    std::atomic<bool> m_bInvalidated = false;
};
}  // namespace Core