// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>

class ChunkChain
{
public:
	struct Chunk
	{
		const Chunk* next;
	};

	ChunkChain() : first_(nullptr) {}
	ChunkChain(const Chunk* chunk) : first_(chunk) {}

	ChunkChain(ChunkChain&& other) : first_(other.release()) {}
	ChunkChain(ChunkChain& other) = delete;		// no copy constructor
	ChunkChain& operator=(const ChunkChain&) = delete;  // no copy assignment

	~ChunkChain()
	{
		const Chunk* chunk = first_;
		while (chunk)
		{
			const Chunk* next = chunk->next;
			delete[] reinterpret_cast<const uint8_t*>(chunk);
			chunk = next;
		}
	}

	const Chunk* first() const noexcept { return first_; }

	void add(Chunk* chunk) noexcept
	{
		chunk->next = first_;
		first_ = chunk;
	}

	const Chunk* release() noexcept
	{
		const Chunk* first = first_;
		first_ = nullptr;
		return first;
	}

private:
	const Chunk* first_;
};
