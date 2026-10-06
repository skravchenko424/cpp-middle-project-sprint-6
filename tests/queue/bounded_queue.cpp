#include "queue/bounded_queue.hpp"
#include <gtest/gtest.h>


#include <atomic>
#include <thread>

using namespace dispatcher::queue;
using namespace std::chrono_literals;

TEST(BoundedQueueTest, PushAndPopSingleTask) {
    BoundedQueue queue(10);
    bool executed = false;
    queue.push([&executed] { executed = true; });

    auto task = queue.try_pop();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_TRUE(executed);
}

TEST(BoundedQueueTest, TryPopOnEmptyReturnsNullopt) {
    BoundedQueue queue(10);
    auto task = queue.try_pop();
    EXPECT_FALSE(task.has_value());
}

TEST(BoundedQueueTest, PushBlocksWhenFull) {
    BoundedQueue queue(2);
    queue.push([] {});
    queue.push([] {});

    std::atomic<bool> push_completed{false};
    std::thread producer([&queue, &push_completed] {
        queue.push([] {});
        push_completed = true;
    });

    std::this_thread::sleep_for(100ms);
    EXPECT_FALSE(push_completed.load());

    queue.try_pop();
    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(push_completed.load());

    producer.join();
}

TEST(BoundedQueueTest, PreservesFifoOrder) {
    BoundedQueue queue(10);
    std::vector<int> order;
    queue.push([&order] { order.push_back(1); });
    queue.push([&order] { order.push_back(2); });
    queue.push([&order] { order.push_back(3); });

    for (int i = 0; i < 3; ++i) {
        auto task = queue.try_pop();
        ASSERT_TRUE(task.has_value());
        (*task)();
    }
    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
}

TEST(BoundedQueueTest, CapacityOneBlockingBehavior) {
    BoundedQueue queue(1);
    queue.push([] {});

    std::atomic<bool> pushed{false};
    std::thread t([&queue, &pushed] {
        queue.push([] {});
        pushed = true;
    });

    std::this_thread::sleep_for(50ms);
    EXPECT_FALSE(pushed.load());

    // Drain to unblock producer
    auto task1 = queue.try_pop();
    ASSERT_TRUE(task1.has_value());
    std::this_thread::sleep_for(50ms);
    EXPECT_TRUE(pushed.load());

    auto task2 = queue.try_pop();
    EXPECT_TRUE(task2.has_value());
    t.join();
}