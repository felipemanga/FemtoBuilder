#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "types.hpp"

template<typename TaskResult, u32 maxRunners = 8>
class TaskManager {
    using Mutex = std::mutex;
    using Guard = std::lock_guard<Mutex>;

    struct Task {
        TaskResult result;
        std::function<TaskResult()> run;
    };

    std::vector<std::shared_ptr<Task>> done;
    Mutex doneMut;

    std::vector<std::shared_ptr<Task>> queue;
    Mutex queueMut;

    std::vector<std::thread> threads;
    std::atomic_bool isLive = false;
    std::atomic_int pending = 0;

    void init() {
        if (isLive)
            return;
        isLive = true;
        threads.resize(maxRunners);
        for (auto& thread : threads)
            thread = std::thread([=]{run();});
    }


    void run() {
        std::shared_ptr<Task> task;
        while (isLive) {
            if (!task) {
                Guard queueLock{queueMut};
                if (!queue.empty()) {
                    task = queue.front();
                    queue.erase(queue.begin());
                }
            }
            if (!task) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            task->result = task->run();

            {
                Guard doneLock{doneMut};
                done.push_back(task);
                --pending;
            }
            task.reset();
        }
    }

public:

    ~TaskManager() {
        isLive = false;
        for (auto& thread : threads)
            thread.join();
        threads.clear();
    }

    void wait(std::function<void(TaskResult)>&& complete) {
        init();
        while (pending || !done.empty()) {
            std::shared_ptr<Task> task;
            {
                Guard doneLock{doneMut};
                if (!done.empty()) {
                    task = done.front();
                    done.erase(done.begin());
                }
            }
            if (task) {
                complete(std::move(task->result));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }

    void add(std::function<TaskResult()>&& run) {
        ++pending;
        auto task = std::make_shared<Task>();
        task->run = std::move(run);

        {
            Guard queueLock{queueMut};
            queue.push_back(task);
        }
    }
};
