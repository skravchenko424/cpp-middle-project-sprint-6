#include "queue/unbounded_queue.hpp"

#include <functional>
#include <mutex>
#include <queue>
#include <semaphore>

namespace dispatcher::queue {

void UnboundedQueue::push(std::function<void()> task) {
    std::lock_guard lock(mutex_);
    q_.push(std::move(task));
}

std::optional<std::function<void()>> UnboundedQueue::try_pop() {
    std::lock_guard lock(mutex_);
    if (q_.empty()) {
        return std::nullopt;
    }

    auto task = std::move(q_.front());
    q_.pop();
    return task;
}

} // namespace dispatcher::queue