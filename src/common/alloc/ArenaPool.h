// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Arena.h"

/**
 * A fast object pool that uses an Arena as its allocator.
 *  
 * This class is not threadsafe.
 */

template <typename T>
class ArenaPool
{
private:
	struct PoolObject
	{
		union
		{
			PoolObject* nextFree;
			T object;
		};
	};
public:
	ArenaPool(Arena& arena) : arena_(arena), firstFree_(nullptr) {}

	T* get()
	{
		if (firstFree_)
		{
			PoolObject* free = firstFree_;
			firstFree_ = free->nextFree;
			return &free->object;
		}
		return arena_.alloc<T>();
	}

	void free(T* obj)
	{
		PoolObject* p = reinterpret_cast<PoolObject*>(obj);
		p->nextFree = firstFree_;
		firstFree_ = p;
	}

private:
	PoolObject* firstFree_;
	Arena& arena_;
};
