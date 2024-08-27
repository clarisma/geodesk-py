// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "LookupBase.h"
#include <common/util/log.h>

/**
 * Template for classes that look up items in a hashtable using a 64-bit 
 * numeric ID. Note that IDs should have reasonable distribution as they
 * are used as the hash keys.
 * 
 * The following functions must be provided by the derived class:
 * 
 * - static T** next(T* item)
 *   Returns the location of the "next item" pointer in the given item
 * 
 * - static int64_t getId(T* item)
 *   Returns the ID of an item.
 */
template<typename Derived, typename T>
class Lookup : public LookupBase<Derived, T>
{
public:
	Lookup() {}

	Lookup(T** table, int tableSize)
	{
		init(table, tableSize);
	}

	/**
	 * Inserts the given item into the index, using its ID 
	 * (retrieved via getID) as a lookup key.
	 * This function assumes that no item with the same ID
	 * already exists in this index.
	 */
	void insert(T* item)
	{
		size_t slot = Derived::getId(item) % this->tableSize_;
		*Derived::next(item) = this->table_[slot];
		this->table_[slot] = item;
	}

	/**
	 * Returns a pointer to the item with the given ID, or nullptr
	 * if no such item exists.
	 */
	T* lookup(uint64_t id) const
	{
		// LOG("Looking up %llu", id);
		size_t slot = id % this->tableSize_;
		T* item = this->table_[slot];
		while (item)
		{
			if (Derived::getId(item) == id) return item;
			item = *Derived::next(item);
		}
		return nullptr;
	}
};
