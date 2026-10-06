#include "queue/priority_queue.hpp"
#include <gtest/gtest.h>


#include <atomic>
#include <thread>

using namespace dispatcher;
using namespace dispatcher::queue;
using namespace std::chrono_literals;

static std::map<TaskPriority, QueueOptions> defaultOptions() {
    return {
        {TaskPriority::High, {.bounded = true, .capacity = 100}},
        {TaskPriority::Normal, {.bounded = false}},
    };
}

TEST(PriorityQueueTest, PopReturnsHighPriorityFirst) {
    PriorityQueue queue(defaultOptions());
    std::vector<int> order;

    queue.push(TaskPriority::Normal, [&order] { order.push_back(0); });
    queue.push(TaskPriority::High, [&order] { order.push_back(1); });

    auto t1 = queue.pop();
    ASSERT_TRUE(t1.has_value());
    (*t1)();
    auto t2 = queue.pop();
    ASSERT_TRUE(t2.has_value());
    (*t2)();

    EXPECT_EQ(order, (std::vector<int>{1, 0}));
}

TEST(PriorityQueueTest, PopBlocksUntilTaskAvailable) {
    PriorityQueue queue(defaultOptions());
    std::atomic<bool> popped{false};
    std::optional<std::function<void()>> result;

    std::thread consumer([&queue, &popped, &result] {
        result = queue.pop();
        popped = true;
    });

    std::this_thread::sleep_for(100ms);
    EXPECT_FALSE(popped.load());

    queue.push(TaskPriority::Normal, [] {});
    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(popped.load());
    EXPECT_TRUE(result.has_value());

    consumer.join();
}

TEST(PriorityQueueTest, ShutdownUnblocksPop) {
    PriorityQueue queue(defaultOptions());
    std::atomic<bool> popped{false};
    std::optional<std::function<void()>> result;

    std::thread consumer([&queue, &popped, &result] {
        result = queue.pop();
        popped = true;
    });

    std::this_thread::sleep_for(100ms);
    EXPECT_FALSE(popped.load());

    queue.shutdown();
    consumer.join();
    EXPECT_TRUE(popped.load());
    EXPECT_FALSE(result.has_value());
}

TEST(PriorityQueueTest, PopReturnsNulloptOnEmptyAfterShutdown) {
    PriorityQueue queue(defaultOptions());
    queue.push(TaskPriority::High, [] {});
    auto task = queue.pop();
    ASSERT_TRUE(task.has_value());

    queue.shutdown();
    auto empty = queue.pop();
    EXPECT_FALSE(empty.has_value());
}

TEST(PriorityQueueTest, InvalidBoundedCapacityThrows) {
    std::map<TaskPriority, QueueOptions> options = {
        {TaskPriority::High, {.bounded = true, .capacity = 0}},
    };
    EXPECT_THROW(PriorityQueue queue(options), std::invalid_argument);
}