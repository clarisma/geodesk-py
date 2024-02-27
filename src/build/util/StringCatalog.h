// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <memory>
#include <vector>
#include <common/util/Bytes.h>
#include <common/util/Strings.h>

class BuildSettings;
class StringStatistics;

class StringCatalog
{
public:
	StringCatalog();

	void build(const BuildSettings& setings, const StringStatistics& strings);
	
	static const char* CORE_STRINGS[];
	static const int CORE_STRING_COUNT = 5;

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

	static void sortDescending(std::vector<SortEntry>& sorted);
	Entry* lookup(const std::string_view str) const noexcept;
	static void addGlobalString(std::vector<Entry*>& globalStrings, Entry* p);
	void addGlobalString(std::vector<Entry*>& globalStrings, std::string_view str);

	std::unique_ptr<uint8_t[]> arena_;
	const uint32_t* table_;
	uint32_t tableSlotCount_;
};
