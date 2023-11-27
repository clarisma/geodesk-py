// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include <common/util/log.h>


class Task
{
public:

};

template <typename C, typename T>
class TaskEngine
{
public:
	TaskEngine(int maxThreads, int queueSize, bool outputThread) :
		size_(queueSize),
		count_(0),
		head_(0),
		tail_(0),
		shutdown_(false)
	{
		for (int i = 0; i < queueSize; i++)
		{
			tasks_.push_back(T());
		}
		if (outputThread)
		{
			// TODO
		}
		for (int i = 0; i < maxThreads; i++)
		{
			threads_.emplace_back(&TaskEngine::run, this);
		}
	}

	void submit(T task)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ == size_)
		{
			// LOG("Queue full.");
			tasksChanged_.wait(lock);
		}
		tasks_[tail_] = task;
		tail_ = (tail_ + 1) % size_;
		count_++;
		// LOG("%d items in queue.", count_);
		tasksChanged_.notify_one();
	}

	void shutdown()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ != 0)
		{
			tasksChanged_.wait(lock);
		}
		shutdown_ = true;
		head_ = 0;
		tail_ = 0;
		tasksChanged_.notify_all();
	}

	void shutdownNow()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		shutdown_ = true;
		count_ = 0;
		head_ = 0;
		tail_ = 0;
		tasksChanged_.notify_all();
	}

	void awaitCompletion()
	{
		for (std::thread& thread : threads_)
		{
			thread.join();
		}
	}


protected:
	virtual void preProcess(C& context)
	{
		// do nothing
	}

	virtual void process(C& context, T task) = 0;

	virtual void postProcess(C& context)
	{
		// do nothing
	}


private:
	void run()
	{
		C context;
		preProcess(context);

		for (;;)
		{
			T task;
			{
				std::unique_lock<std::mutex> lock(mutex_);
				while (count_ == 0 && !shutdown_)
				{
					tasksChanged_.wait(lock);
				}
				if (shutdown_) break;
				task = tasks_[head_];
				head_ = (head_ + 1) % size_;
				count_--;
				tasksChanged_.notify_one();
			}
			process(context, task);
		}
		postProcess(context);
	}


	std::vector<std::thread> threads_;
	std::vector<T> tasks_;
	std::mutex mutex_;
	std::condition_variable tasksChanged_;
	// int maxThreadCount_;
	// int activeThreadCount_;
	int size_;
	int count_;
	int head_;
	int tail_;
	bool shutdown_;
};
