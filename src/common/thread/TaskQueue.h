// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>

template <typename T>
class TaskQueue
{
public:
    TaskQueue(int size) :
        size_(size),
        count_(0),
        front_(0),
        rear_(0),
    {
        queue_.resize(size);
    }

    
    void post(const T& task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == queueSize_)
        {
            notFull_.wait(lock, [this] { return count_ < queueSize_; });
        }
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

    
    int minimumRemainingCapacity()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // TODO: No lock needed, as long as there aren't multiple consumers
        // count_ will only decrease, so this should be safe without locking
        // But no noticeable performance difference, so we'll leave the lock for now
        return queueSize_ - count_;
    }

    void process()
    {
        while (running_)
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

private:
    std::vector<TaskType> queue_;
    std::mutex mutex_;
    std::condition_variable notEmpty_, notFull_;
    int front_;
    int rear_;
    int queueSize_;
    int count_;
    std::atomic<bool> running_;
};
