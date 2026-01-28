#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = 0);
    ~ThreadPool();

    template<typename Container, typename Func>
    void parallel_for(Container& container, Func func) {
        size_t size = container.size();
        if (size == 0) return;

        size_t tasksPerThread = (size + m_threads.size() - 1) / m_threads.size();
        std::vector<std::future<void>> futures;
        futures.reserve(m_threads.size());

        for (size_t t = 0; t < m_threads.size(); ++t) {
            futures.push_back(enqueue([&container, func, t, tasksPerThread, size]() {
                size_t start = t * tasksPerThread;
                size_t end = std::min(start + tasksPerThread, size);
                for (size_t i = start; i < end; ++i) {
                    func(container[i]);
                }
            }));
        }

        for (auto& future : futures) {
            future.get();
        }
    }

    template<typename Func>
    auto enqueue(Func func) -> std::future<decltype(func())> {
        auto task = std::make_shared<std::packaged_task<decltype(func())()>>(func);
        std::future<decltype(func())> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_tasks.push([task]() { (*task)(); });
        }

        m_condition.notify_one();
        return result;
    }

private:
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    bool m_stop = false;

private:
    void workerThread();
};