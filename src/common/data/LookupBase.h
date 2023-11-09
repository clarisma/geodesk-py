#pragma once
#include <cstdint>
#include <cstring>

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
		memset(table, 0, sizeof(T*) * tableSize);
	}

protected:

	T** table_;
	size_t tableSize_;
};
