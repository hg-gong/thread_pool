#include "ThreadPool.h"
#include <chrono>
#include <vector>
#include <iostream>

int square(int v){
    return v*v;
}


int main(){

    ThreadPool pool(4); // 指定四个工作线程

    // 将任务在池中入队，并返回一个 std::future
    std::vector<std::future<int>> fus;
    for(int i=1; i<=100; i++){
        std::function<int()> f = std::bind(square,i);
        fus.push_back(pool.Add(f)); 
    }

    // 从 future 中获得执行结果
    for(auto& fu:fus){
        std::cout << fu.get() << std::endl;
    }

    return 0;
}