// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "ArenaPool.h"

/**
 * A fast stack that uses an ArenaPool for dynamic memory allocation.
 * The items on the stack are stored in a linked list of chunks with
 * a given size. These chunks are allocated via an ArenaPool that can
 * be shared with other stacks that have the same item type and chunk size. 
 * 
 * This class is not threadsafe; concurrent access must be mediated via a mutex.
 */

template <typename T, size_t CHUNK_SIZE>
class ArenaStack
{
private:
	struct Chunk
	{
		Chunk* prev;
		T items[CHUNK_SIZE];
	};

public:
	using Pool = ArenaPool<Chunk>;

	ArenaStack(Pool& pool) : 
		pool_(pool), 
		top_(nullptr),
		chunkItemCount_(CHUNK_SIZE)
	{
	}

	// TODO: Do we really need an explicit destructor?
	~ArenaStack() { clear(); }

	bool isEmpty() const { return top_ == nullptr; }

	void push(T item)
	{
		if (chunkItemCount_ < CHUNK_SIZE)
		{
			top_->items[chunkItemCount_++] = item;
			return;
		}
		Chunk* newTop = pool_.get();
		newTop->prev = top_;
		top_ = newTop;
		newTop->items[0] = item;
		chunkItemCount_ = 1;
	}

	T pop()
	{
		assert(!isEmpty());	// Check for stack underflow
		T item = top_->items[--chunkItemCount_];
		if (chunkItemCount_ == 0) 
		{
			Chunk* newTop = top_->prev;
			pool_.free(top_);
			top_ = newTop;
			chunkItemCount_ = CHUNK_SIZE;
		}
		return item;
	}

	void clear()
	{
		Chunk* p = top_;
		while (p)
		{
			Chunk* prev = p->prev;
			pool_.free(p);
			p = prev;
		}
		top_ = nullptr;
		chunkItemCount_ = CHUNK_SIZE;
	}

private:
	Chunk* top_;
	int chunkItemCount_;
	ArenaPool<Chunk>& pool_;
};
