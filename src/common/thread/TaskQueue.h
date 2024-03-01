// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <common/thread/Threads.h>
#include <common/util/log.h>

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

    
    void post(Task&& task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == size_)
        {
            /*
            printf("Thread %s: Queue %p full, waiting for space...\n",
                Threads::currentThreadId().c_str(), this);
            */
            notFull_.wait(lock, [this] { return count_ < size_; });
        }
        queue_[rear_] = std::move(task);
        rear_ = (rear_ + 1) % size_;
        count_++;
        notEmpty_.notify_one();
    }


    bool tryPost(Task&& task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == size_) return false;
        queue_[rear_] = std::move(task);
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
        for(;;)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                for (;;)
                {
                    if (!running_)
                    {
                        LOG("Thread %s: Finished processing queue %p.", 
                            Threads::currentThreadId().c_str(), this);
                        return;
                    }
                    if (count_ > 0) break;
                    /*
                    printf("Thread %s: Waiting for tasks in queue %p...\n", 
                        Threads::currentThreadId().c_str(), this);
                    */
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
        LOG("Awaiting completion of queue %p...", this);
        std::unique_lock<std::mutex> lock(mutex_);
        while (count_ != 0)
        {  // Continue to wait as long as there are tasks in the queue
            LOG("There are still %d task(s) in queue %p...", count_, this);
            notFull_.wait(lock);  // Wait for a signal that a task has been completed
        }
        LOG("Queue %p is empty.", this);
    }

    void shutdown()
    {
        LOG("Shutting down queue %p...", this);
        std::unique_lock<std::mutex> lock(mutex_);
        running_ = false;
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
    bool running_;
};
