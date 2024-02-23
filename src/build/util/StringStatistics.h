// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <common/util/protobuf.h>
#include <common/util/Strings.h>


class StringStatistics
{
public:
	using StringCount = int64_t;   // Signed to allow -1 as a marker
	using CounterOfs = uint32_t;

	struct CounterHeader
	{
		CounterOfs next;				   // offset of next counter	
		uint32_t hash;
		StringCount keys;
		StringCount values;

		StringCount total() const { return keys + values; }
		StringCount keyCount() const { return keys; }
		StringCount valueCount() const { return values; }
	};

	struct Counter : public CounterHeader
	{
		ShortVarString string;

		Counter(uint32_t next, uint32_t hash, const ShortVarString* str)
		{
			this->next = next;
			this->hash = hash;
			this->keys = 0;
			this->values = 0;
			this->string.init(str);
		}

		void add(int64_t keys, int64_t values)
		{
			this->keys += keys;
			this->values += values;
		}

		static uint32_t grossSize(uint32_t stringSize)
		{
			uint32_t counterSize = sizeof(CounterHeader) + stringSize;
			return (counterSize + 3) & ~3;
			// TODO: Decide on alignment 4 vs. 8
			// TODO: 4 not portable since we are using 64-bit StringCount
			// Should align on 8 bytes instead?
		}

		std::string_view stringView() const
		{
			return string.toStringView();
		}

		uint32_t stringSize() const
		{
			return string.totalSize();
		}
	};

	class Iterator
	{
	public:
		Iterator(const StringStatistics& stats)
		{
			p_ = stats.arena_.get() + sizeof(uint32_t);
			pEnd_ = stats.p_;
		}

		const Counter* next()
		{
			if (p_ < pEnd_)
			{
				const Counter* current = reinterpret_cast<const Counter*>(p_);
				p_ += current->grossSize(current->string.totalSize());
				return current;
			}
			return nullptr;
		}

	private:
		const uint8_t* p_;
		const uint8_t* pEnd_;
	};

	StringStatistics(uint32_t tableSize, uint32_t arenaSize);

	size_t counterCount() const { return counterCount_; }
	Iterator iter() const { return Iterator(*this); }
	// CounterOfs addString(const uint8_t* bytes, StringCount keys, StringCount values);
	CounterOfs addString(const Counter* pCounter);
	void removeStrings(uint32_t minCount);
	std::unique_ptr<uint8_t[]> takeStrings();
	Counter* counterAt(CounterOfs ofs) const
	{
		assert(arena_.get() + ofs < arenaEnd_);
		return reinterpret_cast<Counter*>(arena_.get() + ofs);
	}
	CounterOfs getCounter(const ShortVarString* str, uint32_t hash);
	CounterOfs getCounter(const ShortVarString* str);

private:
	void clearTable();
	void reset(uint32_t arenaSize);
	// void check() const;

	std::unique_ptr<CounterOfs[]> table_;
	std::unique_ptr<uint8_t[]> arena_;
	const uint8_t* arenaEnd_;
	uint8_t* p_;
	size_t tableSize_;
	size_t counterCount_;
};
