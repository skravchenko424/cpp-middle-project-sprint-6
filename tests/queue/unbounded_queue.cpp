#include "queue/unbounded_queue.hpp"
#include <gtest/gtest.h>


#include <atomic>
#include <thread>

using namespace dispatcher::queue;

TEST(UnboundedQueueTest, PushAndPopSingleTask) {
    UnboundedQueue queue;
    bool executed = false;
    queue.push([&executed] { executed = true; });

    auto task = queue.try_pop();
    ASSERT_TRUE(task.has_value());
    (*task)();
    EXPECT_TRUE(executed);
}

TEST(UnboundedQueueTest, TryPopOnEmptyReturnsNullopt) {
    UnboundedQueue queue;
    auto task = queue.try_pop();
    EXPECT_FALSE(task.has_value());
}

TEST(UnboundedQueueTest, DoesNotBlockOnManyPushes) {
    UnboundedQueue queue;
    constexpr int N = 10000;
    for (int i = 0; i < N; ++i) {
        queue.push([] {});
    }

    int popped = 0;
    while (auto task = queue.try_pop()) {
        ++popped;
    }
    EXPECT_EQ(popped, N);
}

TEST(UnboundedQueueTest, PreservesFifoOrder) {
    UnboundedQueue queue;
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

TEST(UnboundedQueueTest, ConcurrentPushAndPop) {
    UnboundedQueue queue;
    constexpr int N = 5000;
    std::atomic<int> counter{0};

    std::thread producer([&queue] {
        for (int i = 0; i < N; ++i) {
            queue.push([] {});
        }
    });

    std::thread consumer([&queue, &counter] {
        int popped = 0;
        while (popped < N) {
            if (auto task = queue.try_pop()) {
                (*task)();
                ++popped;
                ++counter;
            }
        }
    });

    producer.join();
    consumer.join();
    EXPECT_EQ(counter.load(), N);
}