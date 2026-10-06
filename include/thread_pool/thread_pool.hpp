#pragma once

#include <queue/priority_queue.hpp>
#include <thread>

namespace dispatcher::thread_pool {

class ThreadPool {
    std::shared_ptr<queue::PriorityQueue> queue_;
    std::vector<std::jthread> workers_;
    void run();

public:
    explicit ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t threadCount);
    ~ThreadPool();
};

}  // namespace dispatcher::thread_pool
