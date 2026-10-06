#include "task_dispatcher.hpp"
#include <gtest/gtest.h>

#include <atomic>
#include <future>
#include <thread>

using namespace dispatcher;
using namespace std::chrono_literals;

TEST(TaskDispatcherTest, ExecutesScheduledTask) {
    TaskDispatcher dispatcher(1);
    std::promise<void> done;
    auto future = done.get_future();

    dispatcher.schedule(TaskPriority::Normal, [&done] { done.set_value(); });

    EXPECT_EQ(future.wait_for(2s), std::future_status::ready);
}

TEST(TaskDispatcherTest, ExecutesMultipleTasksOnMultipleThreads) {
    TaskDispatcher dispatcher(4);
    constexpr int N = 100;
    std::atomic<int> counter{0};
    std::promise<void> done;
    auto future = done.get_future();

    for (int i = 0; i < N; ++i) {
        dispatcher.schedule(TaskPriority::Normal, [&counter, &done, N] {
            if (++counter == N) {
                done.set_value();
            }
        });
    }

    EXPECT_EQ(future.wait_for(5s), std::future_status::ready);
    EXPECT_EQ(counter.load(), N);
}

TEST(TaskDispatcherTest, HighPriorityTasksProcessedFirst) {
    auto order = std::make_shared<std::vector<int>>();
    auto done = std::make_shared<std::promise<void>>();
    auto future = done->get_future();

    auto started = std::make_shared<std::promise<void>>();
    auto started_future = started->get_future();

    TaskDispatcher dispatcher(1);

    dispatcher.schedule(TaskPriority::Normal, [started] {
        started->set_value();
        std::this_thread::sleep_for(200ms);
    });

    ASSERT_EQ(started_future.wait_for(2s), std::future_status::ready) << "Worker never picked up the blocking task";

    dispatcher.schedule(TaskPriority::Normal, [order] { order->push_back(0); });
    dispatcher.schedule(TaskPriority::High, [order] { order->push_back(1); });
    dispatcher.schedule(TaskPriority::High, [order, done] {
        order->push_back(2);
        done->set_value();
    });

    ASSERT_EQ(future.wait_for(3s), std::future_status::ready) << "Tasks were not processed in time";

    ASSERT_GE(order->size(), 3u);
    EXPECT_EQ((*order)[0], 1);
    EXPECT_EQ((*order)[1], 2);
    EXPECT_EQ((*order)[2], 0);
}

TEST(TaskDispatcherTest, TaskExceptionDoesNotKillPool) {
    TaskDispatcher dispatcher(1);
    std::promise<void> done;
    auto future = done.get_future();

    dispatcher.schedule(TaskPriority::Normal, [] { throw std::runtime_error("boom"); });
    dispatcher.schedule(TaskPriority::Normal, [&done] { done.set_value(); });

    EXPECT_EQ(future.wait_for(2s), std::future_status::ready);
}

TEST(TaskDispatcherTest, DestructorFinishesPendingTasks) {
    std::atomic<int> counter{0};
    {
        TaskDispatcher dispatcher(2);
        for (int i = 0; i < 50; ++i) {
            dispatcher.schedule(TaskPriority::Normal, [&counter] {
                std::this_thread::sleep_for(1ms);
                ++counter;
            });
        }
        // Destructor runs here, must drain the queue
    }
    EXPECT_EQ(counter.load(), 50);
}