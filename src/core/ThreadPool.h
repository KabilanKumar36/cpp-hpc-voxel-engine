#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include "ThreadSafeQueue.h"

namespace Core {

/**
 * @class ThreadPool
 * @brief Manages a pool of worker threads that consume tasks from a ThreadSafeQueue.
 * Utilizes C++20 std::jthread for automatic joining.
 */
class ThreadPool {
public:
    /**
     * @brief Constructs the thread pool.
     * Defaults to (Hardware Threads - 1) to leave the main thread free.
     */
    ThreadPool() : m_bDone(false) {
        // Calculate optimal thread count
        unsigned int iThreadCt = std::thread::hardware_concurrency();
        if (iThreadCt > 1)
            iThreadCt -= 1;  // Leave one for main thread
        else
            iThreadCt = 1;  // Fallback for single-core

        // Launch workers
        m_objJThreads.reserve(iThreadCt);
        for (unsigned int iCt = 0; iCt < iThreadCt; iCt++) {
            // jthread passes a stop_token automatically
            m_objJThreads.emplace_back([this](std::stop_token st) { worker_loop(st); });
        }
    }

    /**
     * @brief Destructor. Stops all threads and joins them.
     */
    ~ThreadPool() {
        m_bDone = true;

        // 1. Wake up all threads waiting on the queue
        m_ObjQueue.invalidate();

        // 2. Request stop on jthreads (sets stop_token)
        for (auto& objThread : m_objJThreads) {
            objThread.request_stop();
        }

        // 3. jthreads automatically join here on destruction
    }

    /**
     * @brief Submits a void() function/lambda to the pool.
     */
    void submit(std::function<void()> objJob) {
        if (!m_bDone) {
            m_ObjQueue.push(std::move(objJob));
        }
    }

private:
    /**
     * @brief The main loop for worker threads.
     * @param st C++20 Stop Token to check for shutdown requests.
     */
    void worker_loop(std::stop_token st) {
        while (!st.stop_requested() && !m_bDone) {
            std::function<void()> objJob;

            // wait_and_pop blocks until job is available or queue is invalidated
            if (m_ObjQueue.wait_and_pop(objJob)) {
                if (objJob) {
                    objJob();
                }
            } else {
                // If wait_and_pop returns false, queue was invalidated (shutdown)
                break;
            }
        }
    }

    std::vector<std::jthread> m_objJThreads;
    std::atomic<bool> m_bDone;
    ThreadSafeQueue<std::function<void()>> m_ObjQueue;
};

}  // namespace Core