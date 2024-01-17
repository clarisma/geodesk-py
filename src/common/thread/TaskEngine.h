// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "TaskQueue.h"
#include <common/thread/Threads.h>
#include <common/text/Format.h>
#include <common/util/log.h>

template <typename Derived, typename WorkContext, typename WorkTask, typename OutputTask>
class TaskEngine
{
public:
    TaskEngine(int numberOfThreads, int workQueueSize = 0, int outputQueueSize = 0) :
        workQueue_(workQueueSize == 0 ? (numberOfThreads * 2) : workQueueSize),
        outputQueue_(outputQueueSize == 0 ? (numberOfThreads * 2) : outputQueueSize)
    {
        workContexts_.reserve(numberOfThreads);
        threads_.reserve(numberOfThreads+1);
        threads_.emplace_back(&TaskEngine::processOutput, this);
        for (int i = 0; i < numberOfThreads; i++)
        {
            workContexts_.emplace_back(reinterpret_cast<Derived*>(this));
            threads_.emplace_back(&TaskEngine::process, this, &workContexts_[i]);
        }
    }

    ~TaskEngine()
    {
        // TODO: Check if already shut down (more efficient)
        end();
    }


    void end()
    {
        LOG("Shutting down workQueue (%p) ...", &workQueue_);
        workQueue_.awaitCompletion();
        workQueue_.shutdown();
        LOG("Waiting for worker threads to end...");
        for (auto it = threads_.begin() + 1; it != threads_.end(); ++it) 
        {
            LOG("  Waiting for worker thread %s...", Format::format(it->get_id()).c_str());
            if (it->joinable()) it->join();
            LOG("    Ended.");
        }

        // Now that all worker threads are completed, all remaining output
        // tasks are enqueued. We'll wait for the output thread to pick
        // up the output tasks, then mark the queue as finished and wait
        // for the output thread to end

        LOG("Shutting down outputQueue (%p)...", &outputQueue_);
        outputQueue_.awaitCompletion();
        outputQueue_.shutdown();
        LOG("Waiting for output thread (%s)...", Format::format(threads_[0].get_id()).c_str());
        if (threads_[0].joinable()) threads_[0].join();
        LOG("  Ended.");

    }

    void postOutput(OutputTask&& task)
    {
        outputQueue_.post(std::move(task));
    }

protected:
    void postWork(WorkTask&& task)
    {
        workQueue_.post(std::move(task));
    }

private:
    void process(WorkContext* ctx)
    {
        workQueue_.process(ctx);
        ctx->afterTasks();
    }
    
    void processOutput()
    {
        outputQueue_.process((Derived*)this);
    }


	std::vector<std::thread> threads_;
    std::vector<WorkContext> workContexts_;
	TaskQueue<WorkContext, WorkTask> workQueue_;
	TaskQueue<Derived, OutputTask> outputQueue_;
};

