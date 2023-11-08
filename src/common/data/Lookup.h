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
		int slot = this->self().getId(item) % this->tableSize_;
		*this->self().next(item) = this->table_[slot];
		table_[slot] = item;
	}

	T* lookup(uint64_t id)
	{
		int slot = id % this->tableSize_;
		T* item = table_[slot];
		while (item)
		{
			if (this->self().getId(item) == id) return item;
			item = *this->self().next(item);
		}
		return nullptr;
	}

protected:
	// int64_t getId(T* item)

private:
	T** table_;
	int tableSize_;
};
