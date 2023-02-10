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


/**
 * @brief
 * 线程池支持特性
 * 1.指定工作线程数
 * 2.添加工作任务
 * 3.异步返回任务结果
 *
 */
class ThreadPool
{
private:
    /* data */
    std::vector<std::thread> workers;
    std::queue<std::packaged_task<int()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;

public:
    /**
     * @brief 初始化并运行消费者线程
     * 
     * @param v 
     */
    ThreadPool(int v);
    ~ThreadPool();

    /**
     * @brief 往任务队列中添加任务,并返回期物
     * 
     */
    std::future<int> Add(std::function<int()>&);
};

