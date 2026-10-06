#pragma once
#include "queue/queue.hpp"
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<std::function<void()>> q_;
    std::mutex mutex_;

public:
    explicit UnboundedQueue() = default;

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~UnboundedQueue() override = default;
};

}  // namespace dispatcher::queue