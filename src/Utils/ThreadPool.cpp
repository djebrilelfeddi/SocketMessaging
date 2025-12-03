
#include "Utils/ThreadPool.hpp"

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { workerThread(); });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();
    
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void ThreadPool::workerThread() {
    while (!stop) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            
            if (stop && tasks.empty()) {
                return;
            }
            
            if (!tasks.empty()) {
                task = std::move(tasks.front());
                tasks.pop();
            }
        }
        
        if (task) {
            task();
        }
    }
}