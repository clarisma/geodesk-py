// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Validator.h"
#include <algorithm> 
#include <memory>
#include "GolBuilder.h"


// struct Feature



int Validator::quadrant(int col, int row)
{
	return (col & 1) | ((row & 1) << 1);
}

int Validator::quadrant(Tile tile)
{
	return quadrant(tile.column(), tile.row());
}

Validator::Worker::Worker(Validator* validator) :
	validator_(validator),
	thread_(&Validator::process, validator, this),
	data_(256 * 1024, 8)
{
	// Console::debug("Created worker");
}


Validator::Validator(GolBuilder* builder) :
	builder_(builder),
	queue_(builder->threadCount() * 2),
	workPerTile_ (builder->phaseWork(GolBuilder::Phase::VALIDATE) 
		/ builder->tileCatalog().tileCount())
{
	
}

void Validator::process(Worker* worker)
{
	try
	{
		// Console::debug("Validator: Processing queue...");
		queue_.process(worker);
	}
	catch (std::exception& ex)
	{
		// TODO
		Console::msg(ex.what());
	}
}

void Validator::Worker::processTask(TaskKey task)
{
	// Console::debug("Validating %s...", task.tile().toString().c_str());
	// TODO
	validator_->builder_->featurePiles().load(task.pile(), data_);
	validator_->markCompleted(task);
	// Console::debug("  Validated %s", task.tile().toString().c_str());
}

void Validator::markCompleted(TaskKey task)
{
	// This method can be called from multiple worker threads,
	// therefore we need to synchronize access via the mutex

	std::unique_lock<std::mutex> lock(mutexTasks_);
	builder_->progress(workPerTile_);
}

void Validator::validate()
{
	builder_->console().setTask("Validating...");
	int tileCount = builder_->tileCatalog().tileCount();
	std::unique_ptr<uint64_t> tasks(new uint64_t[tileCount]);
	for (int i = 0; i < tileCount; i++)
	{
		tasks.get()[i] = TaskKey(builder_->tileCatalog().tileOfPile(i), i+1);
		// Pile numbers start at 1, not 0
	}
	std::sort(tasks.get(), tasks.get() + tileCount);

	int threadCount = builder_->threadCount();
	workers_.reserve(threadCount);
	for (int i = 0; i < threadCount; i++)
	{
		// Console::debug("Creating worker #%d...", i);
		workers_.emplace_back(this);
	}


	const uint64_t* p = tasks.get();
	int batch = TaskKey(*p).batch();
	for (int i = 0; i < tileCount; i++)
	{
		TaskKey task = *p++;
		queue_.post(std::move(task));
	}
	queue_.awaitCompletion();
	queue_.shutdown();
	for (auto it = workers_.begin(); it != workers_.end(); ++it)
	{
		it->join();
	}
	assert(_CrtCheckMemory());
}
