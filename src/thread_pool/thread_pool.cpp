#include "thread_pool/thread_pool.hpp"

#include <logger.hpp>

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count) : queue_(std::move(queue)) {
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { run(); });
    }
}

void ThreadPool::run() {
    while (auto task = queue_->pop()) {
        try {
            (*task)();
        } catch (...) {
            Logger::Get().Log("Task stopped with exception");
        }
    }
}

ThreadPool::~ThreadPool() {
    queue_->shutdown();
    for (auto &worker : workers_) {
        worker.join();  // to finish threads before thread pool members deinit
    }
}

}  // namespace dispatcher::thread_pool