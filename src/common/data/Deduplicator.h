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
		*this->self().next(item) = this->table_[slot];
		this->table_[slot] = item;
	}

	T* insert(T* item)
	{
		int itemLen = this->self().length(item);
		void* itemData = this->self().data(item);
		int slot = hash(item) % this->tableSize_;
		T* existing = this->table_[slot];
		while (existing)
		{
			int existingLen = this->self().length(existing);
			if (existingLen == itemLen && 
				memcmp(itemData, this->self().data(existing), itemLen) == 0)
			{
				return existing;
			}
			existing = *this->self().next(existing);
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
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data(item));
		const uint8_t* end = p + length(item);
		do
		{
			hash = hash * 31 + *p++;
		}
		while (p < end);
		return hash;
	}
};
