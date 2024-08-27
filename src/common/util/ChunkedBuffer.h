// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/alloc/Arena.h>
#include "Buffer.h"

class ChunkedBuffer : public Buffer
{
private:
	struct Chunk;

public:
	ChunkedBuffer(Arena& arena, uint32_t grossChunkSize) :
		arena_(arena)
	{
		Chunk* chunk = arena.allocWithExplicitSize<Chunk>(grossChunkSize);
		buf_ = p_ = reinterpret_cast<char*>(chunk->data);
		end_ = buf_ + grossChunkSize - sizeof(ChunkHeader);
		chunk->next = chunk;
	}

	void filled(char* p) override
	{
		size_t netCapacity = end_ - buf_;
		Chunk* prev = reinterpret_cast<Chunk*>(buf_ - sizeof(ChunkHeader));
		Chunk* next = arena_.allocWithExplicitSize<Chunk>(netCapacity + sizeof(ChunkHeader));
		next->next = prev->next;
		prev->next = next;
		buf_ = p_ = reinterpret_cast<char*>(next->data);
		end_ = buf_ + netCapacity;
	}

	void flush(char* p) override
	{
		p_ = p;
	}

	template<typename T>
	void writeTo(T* out)
	{
		size_t netCapacity = end_ - buf_;
		Chunk* current = reinterpret_cast<Chunk*>(buf_ - sizeof(ChunkHeader));
		Chunk* chunk = current->next;
		for (;;)
		{
			if (chunk == current)
			{
				out->write(chunk->data, p_ - buf_);
				break;
			}
			else
			{
				out->write(chunk->data, end_ - buf_);
			}
			chunk = chunk->next;
		}
	}

private:
	struct ChunkHeader
	{
		Chunk* next;
	};

	struct Chunk : ChunkHeader
	{
		uint8_t data[1];		// variable length
	};

	Arena& arena_;
};
