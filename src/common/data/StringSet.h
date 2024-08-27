// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "common/util/Bytes.h"
#include "StringList.h"
#include "StringLookup.h"

template<typename S>
class StringSet;

/**
 * A fast and memory-efficient set of strings. The set is bulk-loaded at 
 * construction and is immutable. Its total size (hashtable and actual 
 * strings) may not exceed 4 GB.
 * 
 * The strings can be TinyString, ShortString or ShortVarString objects.
 * It is the caller's responsibility that the provided strings meet
 * the size limitations.
 */
template<typename S>
class StringSet : public StringLookup<StringSet<S>, S, StringSet::Entry>
{
public:
	template<typename LT>
	StringSet(const StringList<LT>& list)
	{
		// first, we calculate the total size taken up by the entries

		size_t count = list.size();
		size_t stringAreaSize = 0;
		for(int i=0; i<count; i++)
		{
			stringAreaSize += entrySize(list.get(i).length());
		}

		// Now we select a reasonable table size (twice the number
		// of items, rounded up to the nearest power-of-2)

		tableSize_ = Bytes::roundUpToPowerOf2(count * 2);
		uint32_t tableSizeInBytes = tableSize_ * sizeof(uint32_t);
		data_ = new uint8_t[tableSizeInBytes + stringAreaSize];
		clearTable();

		// Finally, we create the entries and put them into the
		// hashtable

		uint32_t* pb = data_.get() + tableSizeInBytes;
		for(int i=0; i<count; i++)
		{
			Entry* pe = reinterpret_cast<Entry*>(pb);
			pe->next = 0;
			std::string_view s = list.get(i);
			pe->string.init(s.data(), s.length());
			insert(pe);
			pb += entrySize(p->length());
			p++;
		}
	}

	template<typename LS>
	bool contains(LS str)
	{
		return lookup(str) != nullptr;
	}

	template<typename LS>
	bool contains(uint32_t hash, LS str)
	{
		return lookup(hash, str) != nullptr;
	}

	Entry
	{
		uint32_t next;
		S string;
	};

	static uint32_t entrySize(uint32_t stringLen)
	{
		return Bytes::aligned(
			sizeof(Entry) - sizeof(S) + S::totalSize(stringLen),
			alignof(Entry));
	}

	static const S& string(T* p) noexcept   // CRTP override
	{
		return p->string;
	}
 
	static uint32_t& next(T* p) noexcept   // CRTP override
	{
		return p->next;
	}
 
	uint32_t[] *table() const noexcept   // CRTP override
	{
		return reinterpret_cast<uint32_t*>(data_.get());
	}
 
	uint32_t tableSize() const noexcept   // CRTP override
	{
		return tableSize_;
	}
 
	void* data() const noexcept   // CRTP override
	{
		return data_.get();
	}
 
private:
	std::unque_ptr<uint8_t> data_;
	uint32_t_ tableSize_;
};
