// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Validator.h"
#include <algorithm> 
#include <memory>
#include <common/util/varint.h>
#include "build/GolBuilder.h"
#include "build/util/ProtoGol.h"
#include "geom/Box.h"

namespace Validator {

class Feature
{
protected:
	Feature(uint64_t idAndFlags, uint32_t next) :
		idAndFlags_(idAndFlags),
		next_(next)
	{
	}

protected:
	uint64_t idAndFlags_;
	union
	{
		// Slot of next feature whose ID has the same hash
		// - Invalid once TEX has been assigned
		uint32_t next_;
		// TEX (for an exported feature)
		int tex_;
	};
	// Slot in nodes where the next node is stored whose coordinates
	// have the same hash (oly used by nodes)
	struct
	{
		uint32_t nextAtLocation_;
		uint32_t exportTilesSlot_;
	};
};

class Node : public Feature
{
public:
	Node(int64_t id, Coordinate xy, uint32_t next) :
		Feature(id << 8, next),
		xy_(xy)
	{
	}

private:
	Coordinate xy_;
};

class Way : public Feature
{
	// Offset in the proto-tile data where the nodeIds (for way) or
	// members (for relation) are stored; not used by node (which 
	// uses nextAtLocation_ instead)
	// - Only used by local features
	uint32_t bodyOfs_;

	// Slot in bounds where the feature's bounding box is stored
	uint32_t bounds_;
};

struct Bounds
{
	Box box;
	uint64_t exportTiles;
};


int quadrant(int col, int row)
{
	return (col & 1) | ((row & 1) << 1);
}

int quadrant(Tile tile)
{
	return quadrant(tile.column(), tile.row());
}

Worker::Worker(Validator* validator) :
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

void Worker::processTask(TaskKey task)
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


void Worker::readTile()
{
	const uint8_t* p = data_.data();
	const uint8_t* pEnd = p + data_.size();

	while (p < pEnd)
	{
		int groupMarker = *p++;
		int groupType = groupMarker & 7;
		int featureType = groupMarker >> 3;
		if (groupType == ProtoGol::GroupType::LOCAL_GROUP)
		{
			if (featureType == ProtoGol::FeatureType::NODES)
			{
				readNodes(p);
			}
			else if (featureType == ProtoGol::FeatureType::WAYS)
			{
				readWays(p);
			}
			else if (featureType == ProtoGol::FeatureType::RELATIONS)
			{
				readRelations(p);
			}
			else
			{
				// TODO: Log.error("Unknown marker %d in tile %s (Pile %d)", groupMarker,
				//	Tile.toString(sourceTile), sourcePile);
				break;
			}
		}
		else if (groupType == ProtoGol::GroupType::EXPORTED_GROUP)
		{
			switch (featureType)
			{
			case ProtoGol::FeatureType::NODES:
				readForeignNodes(p);
				break;
			case ProtoGol::FeatureType::WAYS:
				readForeignFeatures(p, ways_);
				break;
			case ProtoGol::FeatureType::RELATIONS:
				readForeignFeatures(p, relations_);
				break;
			}
		}
		else
		{
			// TODO: Log.error("Unknown marker %d in tile %s (Pile %d)", groupMarker,
			//	Tile.toString(sourceTile), sourcePile);
			break;
		}
	}
}

void Worker::readNodes(const uint8_t*& p)
{
	int64_t prevId = 0;
	int32_t prevX = 0;
	int32_t prevY = 0;
	for (;;)
	{
		int64_t id = readVarint64(p);
		if (id == 0) break;
		bool isTagged = (id & 1);
		id = prevId + (id >> 1);
		int32_t x = readSignedVarint32(p) + prevX;
		int32_t y = readSignedVarint32(p) + prevY;
		/*
		int pos = nodes.size();
		int nodeFlags = (NODE_HAS_TAGS | NODE_IS_FEATURE) * tagsFlag;
		// since tagsFlags is either 1 or 0, sets or clears both flags
		// without need for branching
		nodes.add((int)(id >> 32) | nodeFlags);
		nodes.add((int)id);
		nodes.add(x);
		nodes.add(y);
		nodes.add(0);
		if (tagsFlag != 0)
		{
			int tagsLen = (int)sourceData.readVarint();
			sourceData.skip(tagsLen);
		}
		assertDoesNotExist(nodeIndex, "node", id);
		nodeIndex.put(id, pos);
		prevId = id;
		prevX = x;
		prevY = y;
		*/
	}
}

void Worker::readWays(const uint8_t*& p)
{
}

void Worker::readRelations(const uint8_t*& p)
{
}

void Worker::readForeignNodes(const uint8_t*& p)
{
}

void Worker::readForeignFeatures(const uint8_t*& p, ReusableBlock& features)
{
}

} // namespace Validator