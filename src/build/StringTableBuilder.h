// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <memory>


/*
 We need to create:
 - A lookup table that tells the Sorter which OSM string has a proto-GOL
   string code for keys and/or values
*/
class ProtoStringLookup
{
public:
	uint32_t keyVarintCode(const uint8_t* str);
	uint32_t valueVarintCode(const uint8_t* str);

private:
	struct EntryHeader
	{
		uint32_t next;
		uint32_t hash;
		uint32_t codes[2];
	};

	struct Entry : public EntryHeader
	{
		uint8_t stringBytes[1];		// variable length
	};

	std::unique_ptr<uint8_t> arena_;
};

class StringTableBuilder
{
public:
	StringTableBuilder();
};
