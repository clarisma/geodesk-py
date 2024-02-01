// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>


/**
 * A simplified arena allocator. Use this class instead of the regular
 * Arena if all of the following conditions can be met:
 * 
 * - All chunks will be the same size (Regular Arena allows incremental growth)
 * - All allocations are sized to ensure that subsequent allocations 
 *   are properly aligned (Regular Arena allow allocations to specify 
 *   their desired alignment)
 * - Allocation sizes never exceed the chunk size (Regular Arena allows
 *   "whales" that are allocated as separate chunks)
 * 
 */

static_assert(nullptr == 0, "nullptr is not represented as 0!");

class SimpleArena
{
public:
	struct Chunk
	{
		const Chunk* next;
	};

	SimpleArena(size_t chunkSize) noexcept :
		current_(nullptr)
	{
		end_ = reinterpret_cast<uint8_t*>(chunkSize + sizeof(Chunk));
		p_ = end_;
		// These pointers don't point to valid memory, but that's ok since
		// they are never dereferenced. This allows us the deduce the chunkSize,
		// while forcing a chunk to be allocated the first time the arena
		// receives an allocation request
		// This allows us to keep the constructor alloc-free (and hence
		// guarantees no excpetion wil be thrown), and we defer allocation
		// until the arena is actually used
	}

	~SimpleArena()
	{
		const Chunk* chunk = current_;
		while (chunk)
		{
			const Chunk* next = chunk->next;
			delete[] reinterpret_cast<const uint8_t*>(chunk);
			chunk = next;
		}
	}

	uint8_t* alloc(size_t size)
	{
		assert(size <= end_ - reinterpret_cast<uint8_t*>(current_) - sizeof(Chunk));
		if (p_ + size > end_) 
		{
			allocChunk();
		}
		uint8_t* pStart = p_;
		p_ += size;
		return pStart;
	}

	const Chunk* release() noexcept
	{
		const Chunk* first = current_;
		current_ = nullptr;
		return first;
	}

private:
	void allocChunk()
	{
		size_t grossSize = end_ - reinterpret_cast<uint8_t*>(current_);
		uint8_t * newChunkRaw = new uint8_t[grossSize];
		p_ = newChunkRaw + sizeof(Chunk);
		end_ = newChunkRaw + grossSize;
		Chunk* newChunk = reinterpret_cast<Chunk*>(newChunkRaw);
		newChunk->next = current_;
		current_ = newChunk;
	}

	Chunk* current_;
	uint8_t* p_;
	uint8_t* end_;
};


