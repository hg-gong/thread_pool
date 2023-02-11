/**
 * @file ThreadPool.h
 * @author Larry
 * @brief 
 * @version 0.1
 * @date 2023-02-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <thread>
#include <memory>
#include <future>
#include <functional>
#include <queue>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <iostream>
#include <type_traits>


/**
 * @brief
 * 线程池支持特性
 * 1.指定工作线程数
 * 2.添加任意参数类型和任意返回类型的任务
 * 3.异步返回任务结果
 */
class ThreadPool
{
private:
    /* data */
    std::vector<std::thread> workers;
    std::queue<std::packaged_task<int()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop;

public:
    /**
     * @brief 初始化并运行消费者线程
     * 
     * @param v 
     */
    ThreadPool(int v);

    /**
     * @brief 当客户段的ThreadPool离开主线程的作用域后，需要一个机制告诉工作线程停止运行，并销毁工作线程，返还资源
     * 当ThreadPool离开作用域后，调用析构函数，设置stop为false
     * 工作线程检查到stop为false时，停止工作
     */
    ~ThreadPool();

    /**
     * @brief 往任务队列中添加任务,并返回期物
     *
     */
    template<typename F, typename... Args>
    auto Add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    
};

ThreadPool::ThreadPool(int v) : stop(false)
{
    for (int i = 0; i < v; i++)
    {
        workers.emplace_back(
            // 此处需要捕获this才能使用类的成员变量
            [this]()
            {
                while (1)
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this]()
                            { return this->stop ||!this->tasks.empty(); }); // 检查任务队列非空，避免虚假唤醒
                    if(this->stop && this->tasks.empty()){
                        std::cout << "woker quit" << std::endl;
                        return;
                    }
                    auto task = std::move(this->tasks.front());
                    this->tasks.pop();
                    task(); // 异步launch task
                }
            });
    }
};

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();
    for (auto &worker : workers)
    {
        worker.join();
    }
};

template <typename F, typename... Args>
auto ThreadPool::Add(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type; // deduce return type
    // packeged function to implemente asycn execution
    auto task = std::packaged_task<return_type()>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    // get future to get the access of the result of asycn execution
    std::future<return_type> result = task.get_future();
    {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace(std::move(task));
    }
    cv.notify_one(); // wake up worker
    return result;
};