// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <filesystem>
#include <common/io/Mappedfile.h>

class MappedIndex
{
public:
	~MappedIndex() { release(); }

	void create(std::filesystem::path filePath, int64_t maxId, int valueWidth);
	void release();
	uint64_t* data() const { return index_; }
	int64_t maxId() const { return maxId_; }
	int valueWidth() const { return valueWidth_; }

	static const int64_t SEGMENT_LENGTH_BYTES = 1024 * 1024 * 1024;		// 1 GB
		// TODO: put this in a common place

private:
	uint64_t calculateMappingSize();

	uint64_t* index_ = nullptr;
	int64_t maxId_ = 0;
	int valueWidth_ = 0;
};
