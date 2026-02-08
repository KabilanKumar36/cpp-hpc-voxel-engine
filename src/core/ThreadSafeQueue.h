#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
namespace Core {
template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    void push(T&& objValue) {
        {
            std::scoped_lock objLock(m_objMutex);
            m_queJobs.push(std::move(objValue));
        }
        m_objContVar.notify_one();
    }
    void push(const T& objValue) {
        {
            std::scoped_lock objLock(m_objMutex);
            m_queJobs.push(objValue);
        }
        m_objContVar.notify_one();
    }

    bool wait_and_pop(T& objValue) {
        std::unique_lock<std::mutex> objLock(m_objMutex);

        m_objContVar.wait(objLock, [this] { return !m_queJobs.empty(); });

        objValue = std::move(m_queJobs.front());
        m_queJobs.pop();

        return true;
    }

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

private:
    std::queue<T> m_queJobs;
    mutable std::mutex m_objMutex;
    std::condition_variable m_objContVar;
};
}  // namespace Core
