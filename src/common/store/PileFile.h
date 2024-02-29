// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only
 
#pragma once
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

	void create(const char* filename, uint32_t pileCount, uint32_t pageSize);
	void append(uint32_t pile, const uint8_t* data, size_t len);
	Data load(uint32_t pile);
		

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

	struct PageHeader
	{
		uint32_t nextPage;
	};

	struct Page : public PageHeader
	{
		uint8_t data[1];			// variable size
	};

	Metadata* metadata() const { return reinterpret_cast<Metadata*>(mainMapping()); }
	uint32_t allocPage() const
	{
		uint32_t page = metadata()->pageCount;
		metadata()->pageCount = page + 1;
		return page;
	}

	Page* getPage(uint32_t page) 
	{
		return reinterpret_cast<Page*>(translate(page << metadata()->pageSizeShift));
	}

	uint32_t pageSize_;
	uint32_t sizeMask_;
};
