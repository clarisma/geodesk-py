// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include <common/util/Strings.h>

// TODO

/**
 *     static size_t hash(const uint8_t* str)
 *     static size_t hash(const T* p)
 *     static const uint8_t* string(const T* p);
 *     static bool hashEquals(T* p, size_t hash)
 *     static bool stringEquals(T* p, const uint_t* str)
 *     static uint32_t& next(T* p)
 *     uint32_t[]* table()
 *     uint32_t tableSize()
 *     T* entryAt(uint32_t ofs)
 */

template<typename Derived, typename S, typename T>
class StringLookupBase
{
public:
	Derived* self() { return reinterpret_cast<Derived*>(this); }
		
	// TODO: this can over-read if string is "", always ensure
	// that the byte after the first length-byte is accessible
	static uint32_t stringSize(const uint8_t* str)
	{
		uint8_t len = *bytes;
		return (len & 0x80) ?
			(((static_cast<uint32_t>(*(bytes + 1)) << 7) |
				(len & 0x7f)) + 2) : (len + 1);
	}

	/* Override this method if the string's hash is stored within
	 * the entry (By default, it is computed from the string)
	 */
	static size_t hash(const T* p)
	{
		return self()->hash(self()->string(p));
	}

	/* 
	 * Checks if the given entry's string *may* match based on the
	 * given hash code.
	 * Override this method if the string's hash is stored within
	 * the entry. If so, during lookup we can compare the hash code 
	 * first and skip any entries whose string content definitely
	 * won't match. 
	 * By default, we force a string comparison for each entry.
	 * 
	 * @return false if p's string is definitely not a mtch, otherwise
	 *         true to idnicate that p's string should be compared
	 */
	static bool hashEquals(T* p, size_t hash)
	{
		return true;
	}

	// TODO: wrong, don't use memcmp, possible overrun
	// since memcmp() may read entire words, therefore it
	// won't stop if string lengths mismatch
	// Could make it safe by ensuring that strings always
	// occupy word-aligned memory
	static bool stringEquals(T* p, const uint_t* str)
	{
		const uint8_t* candidateStr = self()->string(p);
		uint32_t size = self()->stringSize(str);
		return memcmp(str, candidateStr, size) == 0;
	}

	static size_t hash(const uint8_t* str)
	{
		Strings::hash(str, self()->stringSize(str));
	}

	void clearTable()
	{
		uint32_t[] * table = self()->table();
		memset(table, 0, sizeof(uint32_t) self()->tableSize());
	}

	void insert(uint32_t ofs)
	{
		T* entry = self()->entryAt(ofs);
		uint32_t hash = self()->getHash(entry);
		uint32_t slot = static_cast<uint32_t>(hash % self()->tableSize());
		self()->next(entry) = self->table()[slot];
		self->table()[slot] = entry;
	}

	T* lookup(const uint8_t* str)
	{
		size_t hash = self()->hash(str);
		uint32_t slot = static_cast<uint32_t>(hash % self()->tableSize());
		uint32_t ofs = self->table()[slot];
		while (ofs)
		{
			T* p = entryAt(ofs);
			if (self()->hashEquals(p, hash))
			{
				if (self()->stringEquals(p, str))
				{
					return p;
				}
			}
			ofs = self()->next(p);
		}
		return nullptr;
	}
};

template<typename Derived, typename T>
class StringLookup : public StringLookupBase<Derived, T>
{
public:
	struct EntryHeader
	{
		uint32_t next;
		uint32_t hash;
	};

	static size_t getHash(const T* p)
	{
		return p->hash;
	}

	static bool hashEquals(T* p, size_t hash)
	{
		return p->hash == hash;
	}

	static uint32_t& next(T* p)
	{
		return p->next;
	}
}

