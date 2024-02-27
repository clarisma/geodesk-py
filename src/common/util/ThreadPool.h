// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>

template <typename TaskType>
class ThreadPool
{
public:
    ThreadPool(int numberOfThreads, int queueSize) :
        queueSize_(queueSize == 0 ? (numberOfThreads * 4) : queueSize), 
        count_(0), 
        front_(0), 
        rear_(0), 
        running_(true)
    {
        numberOfThreads = (numberOfThreads == 0) ? 1 : numberOfThreads;
        threads_.reserve(numberOfThreads);
        queue_.resize(queueSize_);      // Don't use `queueSize` since we adjusted it
        for (int i = 0; i < numberOfThreads; i++)
        {
            threads_.emplace_back(&ThreadPool::worker, this);
        }
    }

    ~ThreadPool()
    {
        shutdown();
    }

    void post(const TaskType& task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this] { return count_ < queueSize_; });
        queue_[rear_] = task;
        rear_ = (rear_ + 1) % queueSize_;
        count_++;
        notEmpty_.notify_one();
    }

    bool tryPost(const TaskType& task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == queueSize_) return false;
        queue_[rear_] = task;
        rear_ = (rear_ + 1) % queueSize_;
        count_++;
        notEmpty_.notify_one();
        return true;
    }

    bool post(const TaskType& task, bool wait)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == queueSize_)
        {
            if (!wait) return false;
            notFull_.wait(lock, [this] { return count_ < queueSize_; });
        }
        queue_[rear_] = task;
        rear_ = (rear_ + 1) % queueSize_;
        count_++;
        notEmpty_.notify_one();
        return true;
    }

    int minimumRemainingCapacity()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // TODO: No lock needed, as long as there aren't multiple consumers
        // count_ will only decrease, so this should be safe without locking
        // But no noticeable performance difference, so we'll leave the lock for now
        return queueSize_ - count_;
    }

    void awaitCompletion()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (count_ != 0) 
        {  // Continue to wait as long as there are tasks in the queue
            notFull_.wait(lock);  // Wait for a signal that a task has been completed
        }
        // When the loop exits, all tasks have been completed

        // TODO: This does not work, because condition is signaled 
        //  when the thread takes the task from the queue, not when it completes it
        // We need a counter that indicates the number of threads still running
    }

    void shutdown()
    {
        signalShutdown();
        for (auto& th : threads_)
        {
            if (th.joinable())
            {
                th.join();
            }
        }
    }

private:
    void signalShutdown()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        running_ = false;
        notEmpty_.notify_all();
    }

    void worker()
    {
        for(;;)
        {
            TaskType task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                for (;;)
                {
                    if (!running_) return;
                    if (count_ > 0) break;
                    notEmpty_.wait(lock);
                }
                task = std::move(queue_[front_]);
                front_ = (front_ + 1) % queueSize_;
                count_--;
                notFull_.notify_one();
            }
            task();
        }
    }

    std::vector<std::thread> threads_;
    std::vector<TaskType> queue_;
    int front_;
    int rear_;
    int queueSize_;
    int count_;
    std::mutex mutex_;
    std::condition_variable notEmpty_, notFull_;
    bool running_;
};
