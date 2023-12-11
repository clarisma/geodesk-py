#pragma once
#include "TaskQueue.h"

template <typename Derived, typename WorkContext, typename WorkTask, typename OutputTask>
class TaskEngine
{
public:
    TaskEngine(int numberOfThreads, int workQueueSize = 0, int outputQueueSize = 0) :
        workQueue_(workQueueSize == 0 ? (numberOfThreads * 4) : workQueueSize),
        outputQueue_(outputQueueSize == 0 ? (numberOfThreads * 4) : outputQueueSize)
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
        end();
    }


    void end()
    {
        workQueue_.awaitCompletion();
        outputQueue_.awaitCompletion();
        workQueue_.shutdown();
        outputQueue_.shutdown();
        for (auto& th : threads_)
        {
            if (th.joinable())
            {
                th.join();
            }
        }
    }

protected:
    void postWork(const WorkTask& task)
    {
        workQueue_.post(task);
    }

private:
    void process(WorkContext* ctx)
    {
        workQueue_.process(ctx);
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

