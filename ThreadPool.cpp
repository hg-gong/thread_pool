#include "ThreadPool.h"


ThreadPool::ThreadPool(int v){
    for(int i=0; i<v; i++){
        workers.emplace_back(
            // 此处需要捕获this才能使用类的成员变量
            [this](){
                while(1){
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this](){return !this->tasks.empty();}); // 检查任务队列非空，避免虚假唤醒
                    auto task = std::move(this->tasks.front()); 
                    this->tasks.pop();
                    task(); // 异步launch task
                }
            }
        );
    }
};

ThreadPool::~ThreadPool(){
    for(auto& worker:workers){
        worker.join();
    }
};

std::future<int> ThreadPool::Add(std::function<int()>& func){
    // 打包func成packged_task, 使得支持异步执行
    std::packaged_task<int()> task(func);
    // 返回期物
    std::future<int> result = task.get_future();
    {
        // 加入任务队列
        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace(std::move(task));
    }
    cv.notify_one();
    return result;
};
