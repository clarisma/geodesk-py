// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include <common/util/Strings.h>

// TODO

/**
 *     static const S& string(T* p)
 *     static uint32_t& next(T* p)
 *     uint32_t[]* table()
 *     uint32_t tableSize()
 *     void* data()
 */

template<typename Derived, typename S, typename T>
class StringLookup
{
public:
	Derived* self() { return reinterpret_cast<Derived*>(this); }

	template <typename LS>
	T* lookup(const LS& str)
	{
		size_t hash = Strings::hash(str);
		return lookup(hash, str);
	}

	template <typename LS>
	T* lookup(uint32_t hash, const LS& str)
	{
		uint32_t slot = static_cast<uint32_t>(hash % self()->tableSize());
		uint32_t ofs = self->table()[slot];
		while (ofs)
		{
			T* p = entryAt(ofs);
			if (self()->string(p) == str) return p;
			ofs = self()->next(p);
		}
		return nullptr;
	}

	T* entryAt(uint32_t ofs) const
	{
		return reinterpret_cast<T*>(
			reinterpret_cast<uint8_t*>(self()->data()) + ofs);
	}

	void insert(T* p)
	{
		insert(Strings::hash(string(p), p);
	}

	void insert(uint32_t hash, T* p)
	{
		uint32_t slot = static_cast<uint32_t>(hash % self()->tableSize());
		self()->next(p) = self->table()[slot];
		self->table()[slot] = static_cast<uint32_t>(p - self()->data());
	}

	void clearTable()
	{
		uint32_t[] * table = self()->table();
		memset(table, 0, sizeof(uint32_t) * self()->tableSize());
	}
};
