// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Arena.h"

// TODO: The handling of whales looks problematic,
// we'll use a simpler approach for now

// TODO: verify that reduceLastAlloc() and freeLastAlloc() work with whales!

/*

void Arena::allocChunk(size_t size)
{
	// TODO: malloc will only align on machine-word size (8 bytes on 64-bit arch)
	// Need to modify this code if we need stricter alignment

	uint8_t* newChunkRaw;
	Chunk* newChunk;

	if (size > nextSize_)
	{
		Console::msg("Allocating whale with %lld bytes", size);

		// Allocate a single "whale"
		// TODO: alignment of whale is asumed at 8, may need to be stricter
		newChunkRaw = new uint8_t[sizeof(Chunk) + size];
		newChunk = reinterpret_cast<Chunk*>(newChunkRaw);
		if (current_)
		{
			// If another chunk has already been allocated, 
			// keep the current chunk and insert the whale after
			// it, so we can keep filling the unused space
			newChunk->next = current_->next;
			current_->next = newChunk;
		}
		else
		{
			// If no chunk has been allocated so far, the whale
			// becomes the current chunk
			newChunk->next = nullptr;
			current_ = newChunk;
		}
	}
	else
	{
		size = nextSize_;
		Console::msg("Allocating chunk with %lld bytes", size);
		newChunkRaw = new uint8_t[sizeof(Chunk) + size];
		newChunk = reinterpret_cast<Chunk*>(newChunkRaw);
		nextSize_ += nextSize_ >> (intialSizeAndPolicy_ & 0xff);
		newChunk->next = current_;
		current_ = newChunk;
	}
	p_ = newChunkRaw + sizeof(Chunk);
	end_ = p_ + size;
}

*/

void Arena::allocChunk(size_t size)
{
	// TODO: malloc will only align on machine-word size (8 bytes on 64-bit arch)
	// Need to modify this code if we need stricter alignment

	// If size is excessive, do a one-off allocation (a "whale"); otherwise, 
	// we allocate a chunk of nextSize_ bytes, and increase nextSize_ 
	// depending on policy

	// Ideally, if we allocate a whale, we would like to have future allocations
	// use the space in the current block (instead of wasting it), but that
	// approach requires too much complexity for now

	if (size <= nextSize_)
	{
		size = nextSize_;
		nextSize_ += nextSize_ >> (intialSizeAndPolicy_ & 0xff);
		// TODO: nextSize_ = nextSize(nextSize_);
	}
	uint8_t* newChunkRaw = new uint8_t[sizeof(Chunk) + size];
	Chunk* newChunk = reinterpret_cast<Chunk*>(newChunkRaw);
	newChunk->next = current_;
	current_ = newChunk;
	p_ = newChunkRaw + sizeof(Chunk);
	end_ = p_ + size;
	// Console::debug("******** Allocating chunk with %lld bytes, p = %p", size, p_);
}