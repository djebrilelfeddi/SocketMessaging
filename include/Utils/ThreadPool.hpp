/**
 * @file ThreadPool.hpp
 * @brief Pool de threads pour traitement parallèle
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
 * @brief Pool de threads worker pour exécution de tâches
 */
class ThreadPool {
public:
    /**
     * @brief Constructeur
     * @param numThreads Nombre de threads dans le pool
     */
    explicit ThreadPool(size_t numThreads);
    
    /**
     * @brief Destructeur (attend la fin des tâches)
     */
    ~ThreadPool();
    
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    /**
     * @brief Ajoute une tâche à la queue
     * @param task Fonction à exécuter
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