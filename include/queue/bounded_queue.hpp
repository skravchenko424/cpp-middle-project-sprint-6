#pragma once
#include "queue/queue.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {

    std::queue<std::function<void()>> q_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    size_t capacity_;

public:
    explicit BoundedQueue(size_t capacity) : capacity_(capacity) {};

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~BoundedQueue() override = default;
};

}  // namespace dispatcher::queue