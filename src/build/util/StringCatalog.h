// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <memory>
#include <vector>
#include <common/util/Bytes.h>
#include <common/util/Strings.h>

class BuildSettings;
class StringStatistics;

struct ProtoStringCode
{
	enum
	{
		KEY = 0,
		VALUE = 1
	};

	static const uint32_t SHARED_STRING_FLAG = 4;

	ProtoStringCode()
	{
		varints[0] = 0;
		varints[1] = 0;
	}

	ProtoStringCode(uint32_t keyCode, uint32_t valueCode)
	{
		varints[0] = keyCode;
		varints[1] = valueCode;
	}

	void validate(const ShortVarString* str, const uint8_t* stringBase)
	{
		const uint8_t* pStr = reinterpret_cast<const uint8_t*>(str);
		assert(pStr >= stringBase);
		assert(pStr - stringBase < (1 << 29));
		uint32_t encodedLiteral = static_cast<uint32_t>(pStr - stringBase) << 3;
		varints[0] = varints[0] ? varints[0] : encodedLiteral;
		varints[1] = varints[1] ? varints[1] : encodedLiteral;
	}

	uint32_t varints[2];

	uint32_t get(int type) const noexcept { return varints[type]; }
};


class StringCatalog
{
public:
	enum ProtoStringType
	{
		KEY = 0,
		VALUE = 1
	};

	StringCatalog();

	void build(const BuildSettings& setings, const StringStatistics& strings);

	/**
	 * Returns the encoded proto-string for the given string.
	 * The returned value has the following format:
	 *    uint32
	 *      Bit 0-1  Number of bytes used by the varint-encoded value - 1
	 *		Bit 2-31 varint28 that holds the index into the proto-string table,
	 *               with its lowest bit set to 1 as a marker to distinguish
	 *               from non-shared string
	 * See https://github.com/clarisma/gol-spec/blob/main/proto-gol.md#string
	 * 
	 * @param type whether the string is a key or value ("value" includes roles)
	 * 
	 * If the given string is not contained in the proto-string table, this
	 * function returns 0. Not that we can distinguish 0 ("not in table")
	 * from string #0, because the shared-string marker bit of the varint
	 * value will be set (i.e. string #0 is 0x04)
	 */
	ProtoStringCode encodedProtoString(const ShortVarString* str, const uint8_t* stringBase) const;
	
	static const char* CORE_STRINGS[];
	static const int CORE_STRING_COUNT = 5;

private:
	struct EntryHeader
	{
		uint32_t next;
		ProtoStringCode code;
	};

	struct Entry : public EntryHeader
	{
		ShortVarString string;

		void mark(uint32_t v)
		{
			code.varints[0] = v;
		}

		bool isMarked() const
		{
			return code.varints[0];
		}

		static uint32_t totalSize(uint32_t stringSize)
		{
			return static_cast<uint32_t>(sizeof(EntryHeader) +
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
	void createProtoStringCodes(const std::vector<SortEntry>& sorted, int type);

	std::unique_ptr<uint8_t[]> arena_;
	const uint32_t* table_;
	uint32_t tableSlotCount_;
};
