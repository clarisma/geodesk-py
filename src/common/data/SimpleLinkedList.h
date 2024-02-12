// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>

/**
 * A simple single-linked list. Items must have a `next` field and
 * expose it via next() and setNext()
 */
template <typename T>
class SimpleLinkedList
{
public:
	SimpleLinkedList() noexcept : first_(nullptr) {}

	class Iterator
	{
	public:
		Iterator(T* first) : next_(first) {}
		T* next() noexcept
		{
			T* item = next_;
			if (item) next_ = item->next();
			return item;
		}

	private:
		T* next_;
	};

	Iterator* iter() const noexcept
	{
		return Iterator(first_);
	}

	void add(T* item) noexcept
	{
		item->setNext(first_);
		first_ = item;
	}

	void clear() noexcept
	{
		first_ = nullptr;
	}

private:
	T* first_;
};
