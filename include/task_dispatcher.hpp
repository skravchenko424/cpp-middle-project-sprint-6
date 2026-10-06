#pragma once

#include <memory>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
    std::shared_ptr<queue::PriorityQueue> queue_;
    thread_pool::ThreadPool threadPool_;

public:
    TaskDispatcher(const size_t threadCount, const std::map<TaskPriority, queue::QueueOptions> &queueParams = {
                                                 {TaskPriority::High, {.bounded = true, .capacity = 1000}},
                                                 {TaskPriority::Normal, {.bounded = false}}});

    void schedule(TaskPriority priority, std::function<void()> task);

    ~TaskDispatcher();
};

}  // namespace dispatcher