// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only
 
#pragma once
#include <filesystem>
#include <common/alloc/ReusableBlock.h>
#include <common/io/ExpandableMappedFile.h>

class PileFile
{
public:
	PileFile();
	~PileFile() { close(); }

	struct Data
	{
		uint8_t* start;
		uint8_t* end;
	};

	void openExisting(const char* filename);
	void create(const char* fileName, uint32_t pileCount, 
		uint32_t pageSize=(1 << 16), uint32_t preallocatedPages=0);
	void preallocate(int pile, int pages);
	void append(int pile, const uint8_t* data, uint32_t len);
	void load(int pile, ReusableBlock& block);
	void close() { file_.close(); }

	static const int MAX_PILE_COUNT = (1 << 26) - 1;
		

private:
	static const uint32_t MAGIC = 0x454C4950;

	struct IndexEntry
	{
		uint32_t firstPage;
		uint32_t lastPage;
		uint64_t totalPayloadSize;
	};

	struct Metadata
	{
		uint32_t magic;
		uint32_t pageCount;
		uint32_t pileCount;
		uint32_t pageSizeShift;		// only lowest 5 bits used
		IndexEntry index[1];		// variable size
	};

	struct Chunk
	{
		uint32_t payloadSize;
		union
		{
			uint32_t remainingSize;
			uint32_t nextPage;
		};
		uint8_t data[1];			// variable size
	};

	struct ChunkAllocation
	{
		Chunk* chunk;
		uint32_t firstPage;
	};

	static const size_t CHUNK_HEADER_SIZE = 8;
		// Don't use sizeof() because it will include alignments

	Metadata* metadata() const { return reinterpret_cast<Metadata*>(file_.mainMapping()); }
	ChunkAllocation allocChunk(uint32_t minPayload);
	Chunk* getChunk(uint32_t page);

	ExpandableMappedFile file_;
	uint32_t pageSize_;
	int pageSizeShift_;
};
