#pragma once
#include "LookupBase.h"

/**
 * A Lookup-based template class that enables de-duplication of items
 * that are represented by a sequence of bytes (such as strings).
 * 
 * Items can be inserted using two ways:
 * 
 * - void insertUnique(T* item)
 *   Adds an item without checking whether it already exists 
 *   (Use this for items that are known to be unique)
 * 
 * - T* insert(T* item)
 *   Checks whether an item with the same content already exists;
 *   if so, returns it instead
 * 
 * Needs the following functions in the derived class:
 * 
 * - static const void* data(T* item)
 *   Returns a pointer to the raw bytes that represent the given item
 * 
 * - static int length(T* item)
 *   Returns the number of bytes that make up the contents of the given item
 * 
 * - static T** next(T* item)
 *   Returns the location of the "next item" pointer in the given item
 */

template<typename Derived, typename T>
class Deduplicator : public LookupBase<Derived, T>
{
public:
	Deduplicator() : count_(0) {}

	Deduplicator(T** table, int tableSize) :
		count_(0)
	{
		init(table, tableSize);
	}

	void insertUnique(T* item)
	{
		int slot = hash(item) % this->tableSize_;
		*Derived::next(item) = this->table_[slot];
		this->table_[slot] = item;
		count_++;
	}

	/**
	 * Checks if an identical item already exists in this lookup table.
	 * If so, adds to the refcount of the exiting item and returns a pointer 
	 * to it; otherwise, inserts the given item (without touching its refcount)
	 * and returns a pointer to it.
	 */
	T* insert(T* item)
	{
		int itemLen = Derived::length(item);
		void* itemData = Derived::data(item);
		int slot = hash(item) % this->tableSize_;
		T* existing = this->table_[slot];
		while (existing)
		{
			int existingLen = Derived::length(existing);
			if (existingLen == itemLen && 
				memcmp(itemData, Derived::data(existing), itemLen) == 0)
			{
				// Derived::addRef(existing);
				return existing;
			}
			existing = *Derived::next(existing);
		}
		item->next = this->table_[slot];
		this->table_[slot] = item;
		count_++;
		return item;
	}

	size_t count() const { return count_; }

	T** toArray(Arena& arena) const
	{
		return LookupBase<Derived,T>::toArray(arena, count_);
	}

private:
	uint32_t hash(T* item) const
	{
		uint32_t hash = 0;
		const uint8_t* p = reinterpret_cast<const uint8_t*>(Derived::data(item));
		const uint8_t* end = p + Derived::length(item);
		do
		{
			hash = hash * 31 + *p++;
		}
		while (p < end);
		return hash;
	}

	size_t count_;
};
