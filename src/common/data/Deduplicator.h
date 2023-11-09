#pragma once
#include "LookupBase.h"

template<typename Derived, typename T>
class Deduplicator : public LookupBase<Derived, T>
{
public:
	Deduplicator() {}

	Deduplicator(T** table, int tableSize)
	{
		init(table, tableSize);
	}

	void insertUnique(T* item)
	{
		int slot = hash(item) % this->tableSize_;
		*Derived::next(item) = this->table_[slot];
		this->table_[slot] = item;
	}

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
				return existing;
			}
			existing = *Derived::next(existing);
		}
		item->next = this->table_[slot];
		this->table_[slot] = item;
		return item;
	}

protected:
	// const void* data(T* item)
	// int length(T* item)
	
	uint32_t hash(T* item)
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
};
