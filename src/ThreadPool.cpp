#include "ThreadPool.hpp"


ThreadPool::ThreadPool(size_t numThreads) {
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
    }

    m_threads.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        m_threads.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }

    m_condition.notify_all();

    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_condition.wait(lock, [this]() { return m_stop || !m_tasks.empty(); });

            if (m_stop && m_tasks.empty()) {
                break;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }

        task();
    }
}