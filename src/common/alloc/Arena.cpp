// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Arena.h"

void Arena::allocChunk(size_t size)
{
	// TODO: malloc will only align on machine-word size (8 bytes on 64-bit arch)
	// Need to modify this code if we need stricter alignment

	uint8_t* newChunkRaw;
	Chunk* newChunk;

	if (size > nextSize_)
	{
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
		newChunkRaw = new uint8_t[sizeof(Chunk) + size];
		newChunk = reinterpret_cast<Chunk*>(newChunkRaw);
		nextSize_ += nextSize_ >> (intialSizeAndPolicy_ & 0xff);
		newChunk->next = current_;
		current_ = newChunk;
	}
	p_ = newChunkRaw + sizeof(Chunk);
	end_ = p_ + size;
}

