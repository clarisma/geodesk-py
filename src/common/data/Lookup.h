#pragma once
#include "LookupBase.h"

template<typename Derived, typename T>
class Lookup : public LookupBase<Derived, T>
{
public:
	Lookup() {}

	Lookup(T** table, int tableSize)
	{
		init(table, tableSize);
	}

	void insert(T* item)
	{
		size_t slot = Derived::getId(item) % this->tableSize_;
		*Derived::next(item) = this->table_[slot];
		this->table_[slot] = item;
	}

	T* lookup(uint64_t id)
	{
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
