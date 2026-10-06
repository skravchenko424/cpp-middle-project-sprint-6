#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {

void BoundedQueue::push(std::function<void()> task) {
    std::unique_lock lock(mutex_);
    not_full_.wait(lock, [this] { return q_.size() < capacity_; });
    q_.push(std::move(task));
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    std::unique_lock lock(mutex_);
    if (q_.empty()) {
        return std::nullopt;
    }

    auto task = std::move(q_.front());
    q_.pop();
    lock.unlock();
    not_full_.notify_one();
    return task;
}

}  // namespace dispatcher::queue