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
	using CounterOfs = uint32_t;

	class Counter
	{
	public:
		Counter(uint32_t next, uint32_t hash, const ShortVarString* str) :
			next_(next),
			hash_(hash),
			totalCount_(0),
			keyCount_(0)
		{
			string_.init(str);
		}

		static const uint64_t REQUIRED = 1ULL << 63;

		Counter(uint32_t next, uint32_t hash, std::string_view str) :
			next_(next),
			hash_(hash),
			totalCount_(0),
			keyCount_(0)
		{
			string_.init(str.data(), str.length());
		}

		uint64_t totalCount() const { return totalCount_; }
		uint64_t trueTotalCount() const { return totalCount_ & ~REQUIRED; }
		uint64_t keyCount() const { return keyCount_; }
		uint64_t valueCount() const { return totalCount_ - keyCount_; }
		uint32_t hash() const { return hash_; }
		uint32_t next() const { return next_; }
		void setNext(uint32_t next) { next_ = next; }
		const ShortVarString& string() const { return string_; }

		void add(int64_t keys, int64_t values)
		{
			totalCount_ += keys + values;
			keyCount_ += keys;
		}

		void add(const Counter* other)
		{
			totalCount_ += other->totalCount_;
			keyCount_ += other->keyCount_;
		}

		static uint32_t grossSize(uint32_t stringSize)
		{
			uint32_t counterSize = sizeof(Counter) - sizeof(string_) + stringSize;
			return (counterSize + 3) & ~3;
			// TODO: Decide on alignment 4 vs. 8
			// TODO: 4 not portable since we are using 64-bit StringCount
			// Should align on 8 bytes instead?
		}

		uint32_t grossSize() const
		{
			return grossSize(stringSize());
		}

		std::string_view stringView() const
		{
			return string_.toStringView();
		}

		uint32_t stringSize() const
		{
			return string_.totalSize();
		}

	private:
		uint32_t next_;
		uint32_t hash_;
		uint64_t totalCount_;
		uint64_t keyCount_;
		ShortVarString string_;
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
				p_ += current->grossSize();
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
	void addRequiredCounter(std::string_view str);

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
