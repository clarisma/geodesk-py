// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/alloc/ReusableBlock.h>
#include <common/thread/TaskQueue.h>
#include "geom/Tile.h"

class GolBuilder;

class Validator
{
public:
	Validator(GolBuilder* builder);
	void validate();

private:
	static int quadrant(int col, int row);
	static int quadrant(Tile tile);

	class TaskKey
	{
	public:
		TaskKey() : data_(0) {}
		TaskKey(uint64_t data) : data_(data) {}
		TaskKey(Tile tile, int pile) :
			data_(
				(static_cast<uint64_t>(15 - tile.zoom()) << 58) |
				(static_cast<uint64_t>(quadrant(tile)) << 56) |
				(static_cast<uint64_t>(pile) << 32) |
				tile)
		{
		}

		operator uint64_t() const
		{
			return data_;
		}

		Tile tile() const
		{
			return static_cast<uint32_t>(data_);
		}

		int pile() const
		{
			return static_cast<int>(data_ >> 32) & 0xffffff;	// lower 24 bits
		}

		int batch() const
		{
			return static_cast<int>(data_ >> 56);
		}

	private:
		uint64_t data_;
	};


	class Worker
	{
	public:
		Worker(Validator* validator);

		void processTask(TaskKey task);
		void join()
		{
			if (thread_.joinable()) thread_.join();
		}

	private:
		Validator* validator_;		// keep this order
		std::thread thread_;
		ReusableBlock data_;
	};

	void process(Worker* worker);
	void markCompleted(TaskKey task);

	GolBuilder* builder_;
	double workPerTile_;
	TaskQueue<Worker,TaskKey> queue_;
	std::vector<Worker> workers_;

	// A mutex that needs to be held in order to access the status
	// of individual tasks
	std::mutex mutexTasks_;

	friend class Worker;
};
