#include "task_dispatcher.hpp"

namespace dispatcher {

TaskDispatcher::TaskDispatcher(const size_t threadCount, const std::map<TaskPriority, queue::QueueOptions> &queueParams)
    : queue_(std::make_shared<queue::PriorityQueue>(queueParams)), threadPool_(queue_, threadCount) {}

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) {
    queue_->push(priority, std::move(task));
}

TaskDispatcher::~TaskDispatcher() { queue_->shutdown(); }

}  // namespace dispatcher