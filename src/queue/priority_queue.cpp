#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(const std::map<TaskPriority, QueueOptions> &priorityOptions) : active_(true) {
    for (const auto &[priority, option] : priorityOptions) {
        if (option.bounded) {
            if (!option.capacity.has_value() || option.capacity.value() <= 0) {
                throw std::invalid_argument("No valid capacity set for BoundedQueue");
            }
            queues_[priority] = std::make_unique<BoundedQueue>(option.capacity.value());
        } else {
            queues_[priority] = std::make_unique<UnboundedQueue>();
        }
    }
}

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    std::unique_lock lock(mutex_);
    queues_.at(priority)->push(std::move(task));
    lock.unlock();
    cv_.notify_one();
}

std::optional<std::function<void()>> PriorityQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
        for (auto &[priority, queue] : queues_) {
            auto task = queue->try_pop();
            if (task) {
                return task;
            }
        }
        if (!active_) {
            return std::nullopt;
        }
        cv_.wait(lock);
    }
}

void PriorityQueue::shutdown() {
    std::unique_lock<std::mutex> lock(mutex_);
    active_ = false;
    lock.unlock();
    cv_.notify_all();
}

PriorityQueue::~PriorityQueue() { shutdown(); }

}  // namespace dispatcher::queue