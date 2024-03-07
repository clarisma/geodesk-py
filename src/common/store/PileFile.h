// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only
 
#pragma once
#include <filesystem>
#include <common/alloc/ReusableBlock.h>
#include <common/io/ExpandableMappedFile.h>

class PileFile : protected ExpandableMappedFile
{
public:
	PileFile();

	struct Data
	{
		uint8_t* start;
		uint8_t* end;
	};

	void create(std::filesystem::path filePath, uint32_t pileCount, uint32_t pageSize);
	void append(int pile, const uint8_t* data, size_t len);
	void load(int pile, ReusableBlock& block);

	static const int MAX_PILE_COUNT = (1 << 26) - 1;
		

private:
	struct IndexEntry
	{
		uint32_t firstPage;
		uint32_t lastPage;
		uint64_t grossSize;
	};

	struct Metadata
	{
		uint32_t magic;
		uint32_t pageCount;
		uint32_t pileCount;
		uint32_t pageSizeShift;		// only lowest 5 bits used
		IndexEntry index[1];		// variable size
	};

	struct Page
	{
		uint32_t nextPage;
		uint8_t data[1];			// variable size
	};

	static const size_t PAGE_HEADER_SIZE = 4;
		// Don't use sizeof() because it will include alignments

	Metadata* metadata() const { return reinterpret_cast<Metadata*>(mainMapping()); }
	uint32_t allocPage() const
	{
		uint32_t page = metadata()->pageCount;
		metadata()->pageCount = page + 1;
		return page;
	}

	Page* getPage(uint32_t page) 
	{
		return reinterpret_cast<Page*>(translate(
			static_cast<uint64_t>(page) << metadata()->pageSizeShift));
	}

	uint32_t pageSize_;
};
