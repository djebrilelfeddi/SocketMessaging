/**
 * @file ThreadPool.hpp
 * @brief Thread pool for parallel processing
 */

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

/**
 * @class ThreadPool
 * @brief Worker thread pool for task execution
 */
class ThreadPool {
public:
    /**
     * @brief Constructor
     * @param numThreads Number of threads in the pool
     */
    explicit ThreadPool(size_t numThreads);
    
    /**
     * @brief Destructor (waits for tasks to complete)
     */
    ~ThreadPool();
    
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    /**
     * @brief Adds a task to the queue
     * @param task Function to execute
     */
    void enqueue(std::function<void()> task);
    
private:
    void workerThread();
    
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

#endif