// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TaskEngine.h"

/*
template <typename C, typename T>
TaskEngine<C,T>::TaskEngine(int maxThreads, int queueSize, bool outputThread) :
	threads_(maxThreads + outputThread),
	tasks_(queueSize)
{
	if (outputThread)
	{
		// TODO
	}
	for (int i = 0; i < maxThreads; i++)
	{
		threads_.emplace_back(&run, this);
	}
}

template <typename C, typename T>
void TaskEngine<C,T>::submit(T task)
{
	std::unique_lock<std::mutex> lock(mutex_);
	while (count_ == size_)
	{
		tasksChanged_.wait(lock);
	}
	tasks_[head_] = task;
	head_ = (head_ + 1) % size_;
	count_++;
	tasksChanged_.notify_one();
}

template <typename C, typename T>
void TaskEngine<C,T>::run()
{
	C context;
	preProcess(context);

	for(;;)
	{
		T task;
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while (count_ == 0 && !shutdown_)
			{
				tasksChanged_.wait(lock);
			}
			if (shutdown_) break;
			T task = tasks_[head_];
			head_ = (head_ + 1) % size_;
			count_--;
			tasksChanged_.notify_one();
		}
		process(context, task);
	}
	postProcess(context);
}

template <typename C, typename T>
void TaskEngine<C, T>::preProcess(C& context)
{
	// do nothing
}

template <typename C, typename T>
void TaskEngine<C, T>::postProcess(C& context)
{
	// do nothing
}


template <typename C, typename T>
void TaskEngine<C, T>::shutdown()
{
	std::unique_lock<std::mutex> lock(mutex_);
	shutdown_ = true;
	count_ = 0;
	head_ = 0;
	tail_ = 0;
	tasksChanged_.notify_all();
}


template <typename C, typename T>
void TaskEngine<C, T>::awaitCompletion()
{
	for (std::thread thread : threads_)
	{
		thread.join();
	}
}
*/