#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include "ThreadSafeQueue.h"

namespace Core {
class ThreadPool {
public:
    ThreadPool() : m_bDone(false) {
        unsigned int iThreadCt = std::thread::hardware_concurrency() - 1;
        if (iThreadCt < 1)
            iThreadCt = 1;
        for (unsigned int iCt = 0; iCt < iThreadCt; iCt++) {
            m_objJThreads.emplace_back([this] { worker_loop(); });
        }
    }
    ~ThreadPool() {
        m_bDone = true;
        for (unsigned int iCt = 0; iCt < m_objJThreads.size(); iCt++) {
            submit([] {});
        }

        for (auto& objThread : m_objJThreads) {
            if (objThread.joinable()) {
                objThread.request_stop();
                objThread.join();
            }
        }
    }
    void submit(std::function<void()> objJob) { m_ObjQueue.push(std::move(objJob)); }

private:
    void worker_loop() {
        while (!m_bDone) {
            std::function<void()> objJob;
            if (m_ObjQueue.wait_and_pop(objJob)) {
                objJob();
            }
        }
    }
    std::vector<std::jthread> m_objJThreads;
    std::atomic<bool> m_bDone;
    ThreadSafeQueue<std::function<void()>> m_ObjQueue;
};

}  // namespace Core