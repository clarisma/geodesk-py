// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>

template <typename Context, typename Task>
class TaskQueue
{
public:
    TaskQueue(int size) :
        size_(size),
        count_(0),
        front_(0),
        rear_(0),
        running_(true)
    {
        queue_.resize(size);
    }

    
    void post(const Task& task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == size_)
        {
            notFull_.wait(lock, [this] { return count_ < size_; });
        }
        queue_[rear_] = task;
        rear_ = (rear_ + 1) % size_;
        count_++;
        notEmpty_.notify_one();
    }


    bool tryPost(const Task& task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == size_) return false;
        queue_[rear_] = task;
        rear_ = (rear_ + 1) % size_;
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
        return size_ - count_;
    }

    void process(Context* ctx)
    {
        while (running_)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                for (;;)
                {
                    if (!running_) return;
                    if (count_ > 0) break;
                    notEmpty_.wait(lock);
                }
                task = std::move(queue_[front_]);
                front_ = (front_ + 1) % size_;
                count_--;
                notFull_.notify_one();
            }
            ctx->processTask(task);
        }
    }


    void awaitCompletion()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (count_ != 0)
        {  // Continue to wait as long as there are tasks in the queue
            notFull_.wait(lock);  // Wait for a signal that a task has been completed
        }
    }

    void shutdown()
    {
        running_.store(false);
        notEmpty_.notify_all();
    }

private:
    std::vector<Task> queue_;
    std::mutex mutex_;
    std::condition_variable notEmpty_, notFull_;
    int front_;
    int rear_;
    int size_;
    int count_;
    std::atomic<bool> running_;
};
