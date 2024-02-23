// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <memory>
#include <common/util/Bytes.h>
#include <common/util/Strings.h>

class BuildSettings;
class StringStatistics;

class StringManager
{
public:
	StringManager();

	void build(const BuildSettings& setings, const StringStatistics& strings);

	/*
	static const uint8_t NO_BYTES[];
	static const ShortVarString& const NO = ShortVarString::cast(NO_BYTES);
	*/

private:
	struct Entry
	{
		uint32_t next;
		uint32_t keyCode;
		uint32_t valueCode;
		ShortVarString string;

		static uint32_t totalSize(uint32_t stringSize)
		{
			return static_cast<uint32_t>(
				sizeof(Entry) - sizeof(ShortVarString) +
				Bytes::aligned(stringSize, alignof(Entry)));
		}

		uint32_t totalSize() const noexcept
		{
			return totalSize(string.totalSize());
		}
	};

	using SortEntry = std::pair<uint64_t, Entry*>;

	void sortDescending(SortEntry* start, SortEntry* end);

	std::unique_ptr<uint8_t[]> arena_;
};
