// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "TaskQueue.h"
#include "TaskStatus.h"
#include <common/thread/Threads.h>
#include <common/text/Format.h>
#include <common/util/log.h>

// TODO: Get rid of the vectors, just use simple arrays?
// --> No, leaves memory uninitialized in the event of an exception
//     --> destructors will fail

// TODO: It would be better to defer initialization of the per-thread 
// WorkContext object; this way, memory used by the main class is
// allocated first. By allocating per-thread memory afterwards, we
// make it more likely that the process gives back this memory to
// the OS when the threads finish and their working set is freed

/**
 * - Before a thread shuts down, it calls afterTasks() on its context
 *   to perform any post-processing that should run concurrently (i.e.
 *   it cannot mutate shared state, at least not without explicit 
 *   synchornization)
 * - After all threads have shut down, harvestResults() is called on
 *   each context to merge any accumulated work. harvestResults() is
 *   synchronous, i.e. it can safely mutate shared state.
 * 
 */

// The constructor of a Context must not access Derived, because
// Derived won't be fully initialzied at that point
// Use a "start" method that inits the contexts and starts the threads

template <typename Derived, typename WorkContext, typename WorkTask, typename OutputTask>
class TaskEngine
{
public:
    TaskEngine(int numberOfThreads, int workQueueSize = 0, int outputQueueSize = 0) :
        threadCount_(numberOfThreads),
        workQueue_(workQueueSize == 0 ? (numberOfThreads * 2) : workQueueSize),
        outputQueue_(outputQueueSize == 0 ? (numberOfThreads * 2) : outputQueueSize)
    {
        assert(numberOfThreads >= 1);
        workContexts_.reserve(numberOfThreads);
        threads_.reserve(numberOfThreads + 1);
    }

    ~TaskEngine()
    {
        // If thread list is empty, this means processing has already ended
        if(!threads_.empty()) end();
    }

    void start()
    {
        threads_.emplace_back(&TaskEngine::processOutput, this);
        for (int i = 0; i < threadCount_; i++)
        {
            workContexts_.emplace_back(reinterpret_cast<Derived*>(this));
            threads_.emplace_back(&TaskEngine::process, this, &workContexts_[i]);
        }
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
        threads_.clear();
        for (auto& ctx : workContexts_)
        {
            ctx.harvestResults();
        }
        workContexts_.clear();
        
    }

    void postOutput(OutputTask&& task)
    {
        outputQueue_.post(std::move(task));
    }

    void reportOutputQueueSpace()
    {
        // printf("%d slots free in output queue\n", outputQueue_.minimumRemainingCapacity());
    }

protected:
    void postWork(WorkTask&& task)
    {
        workQueue_.post(std::move(task));
    }

private:
    void process(WorkContext* ctx)
    {
        try
        {
            workQueue_.process(ctx);
            // Console::debug("Calling afterTasks()...");
            ctx->afterTasks();
        }
        catch (std::exception& ex)
        {
            // TODO
            printf(ex.what());
        }
    }
    
    void processOutput()
    {
        outputQueue_.process((Derived*)this);
    }

	std::vector<std::thread> threads_;
    std::vector<WorkContext> workContexts_;
	TaskQueue<WorkContext, WorkTask> workQueue_;
	TaskQueue<Derived, OutputTask> outputQueue_;
    TaskStatus status_;
    int threadCount_;
};

