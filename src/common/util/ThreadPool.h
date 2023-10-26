// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

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

    void shutdown()
    {
        running_.store(false);
        notEmpty_.notify_all();
        for (auto& th : threads_)
        {
            if (th.joinable())
            {
                th.join();
            }
        }
    }

private:
    void worker()
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

    std::vector<std::thread> threads_;
    std::vector<TaskType> queue_;
    int front_;
    int rear_;
    int queueSize_;
    int count_;
    std::mutex mutex_;
    std::condition_variable notEmpty_, notFull_;
    std::atomic<bool> running_;
};
