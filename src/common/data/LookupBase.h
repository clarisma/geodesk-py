// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <cstring>
#include <common/alloc/Arena.h>

/**
 * Base template for classes that use hashtable-based lookup.
 * Items whose hashes collide are chained using an intrusive linked list,
 * i.e. the pointers to the next item are stored in each item itself.
 * 
 * The hashtable itself is stored externally. Its lifetime is managed
 * by the user, and its pointer and size (number of slots) must be provided
 * to the constructor. (The constructor does, however, initialize the table.)
 * 
 * The following functions must be provided by the derived class:
 * 
 * - static T** next(T* item)
 *   Returns the location of the "next item" pointer in the given item
 */
template<typename Derived, typename T>
class LookupBase
{
public:
	LookupBase() :
		table_(nullptr),
		tableSize_(0)
	{
	}

	void init(T** table, size_t tableSize)
	{
		table_ = table;
		tableSize_ = tableSize;
		clear();
	}

	void clear()
	{
		memset(table_, 0, sizeof(T*) * tableSize_);
	}

	class Iterator
	{
	public:
		Iterator(const LookupBase* lookup)
		{
			pCurrentSlot_ = lookup->table_;
			pEndSlot_ = lookup->table_ + lookup->tableSize_;
			nextSlot();
		}

		bool hasNext()
		{
			return pCurrentItem_ != nullptr;
		}
		
		T* next()
		{
			T* p = pCurrentItem_;
			pCurrentItem_ = *Derived::next(p);
			if (!pCurrentItem_)
			{
				pCurrentSlot_++;
				nextSlot();
			}
			return p;
		}

	private:
		void nextSlot()
		{
			while(pCurrentSlot_ != pEndSlot_)
			{
				pCurrentItem_ = *pCurrentSlot_;
				if (pCurrentItem_) break;
				pCurrentSlot_++;
			}
		}

		T** pCurrentSlot_;
		T** pEndSlot_;
		T* pCurrentItem_;
	};

	Iterator iter() const
	{
		return Iterator(this);
	}

	void copyItems(T** pa)
	{
		Iterator iter(this);
		for (;;)
		{
			T* p = iter.next();
			if (!p) break;
			*pa++ = p;
		}
	}

	T** toArray(Arena& arena, size_t count) const
	{
		T** pa = arena.allocArray<T*>(count);
		T** p = pa;
		T** pEnd = pa + count;
		Iterator iter(this);
		while (p < pEnd)
		{
			T* item = iter.next();
			assert(item);
			*p++ = item;
		}
		return pa;
	}

protected:

	T** table_;
	size_t tableSize_;
};
