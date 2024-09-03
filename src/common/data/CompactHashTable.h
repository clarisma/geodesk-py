// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/data/Span.h>
#include <cstring>
#include <functional>
#include <vector>

template<typename Derived, typename T, typename K>
class CompactHashTable
{
public:
	static const T* lookup(const Span<const T> table, const K& k)
	{
		size_t hash = std::hash<K>()(k);
		size_t slot = hash % table.size();
		do
		{
			const T& item = table[slot];
			if (Derived::key(item) == k) return &item;
			slot = Derived::next(item);
		}
		while (slot != END);
		return nullptr;
	}

	static void create(Span<T> table, Span<T> source)
	{
		assert(table.size() == source.size());
		size_t size = source.size();
		memset(table.data(), 0, size * sizeof(T));
			// TODO: need better init

		// First, place the items that don't collide

		for (int i = 0; i < size; i++)
		{
			T& item = source[i];
			size_t hash = std::hash<K>()(Derived::key(item));
			size_t slot = hash % size;
			if (Derived::next(table[slot]) == 0)
			{
				Derived::setNext(item, END);
				table[slot] = item;
			}
			else
			{
				Derived::setNext(item, slot);
			}
		}

		// Next, create a linked list of all the empty slots

		size_t firstEmpty = END;
		for (int i = 0; i < size; i++)
		{
			T& item = table[i];
			if (Derived::next(item) == 0)
			{
				Derived::setNext(item, firstEmpty);
				firstEmpty = i;
			}
		}

		// finally, place the collision items into the empty slots

		int i = 0;
		while (firstEmpty != END)
		{
			T& item = source[i];
			size_t slot = Derived::next(item);
			if (slot != END)
			{
				Derived::setNext(item, Derived::next(table[slot]));
				Derived::setNext(table[slot], firstEmpty);
				uint32_t nextEmpty = Derived::next(table[firstEmpty]);
				table[firstEmpty] = item;
				firstEmpty = nextEmpty;
			}
			i++;
		}
		assert(i <= size);
	}

private:
	static constexpr uint32_t END = 0xffff'ffff;
};
