// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>

template <typename T>
class Linked
{
public:
	T* next() const { return next_; }

protected:
	Linked(T* next) : next_(next) {}

public:				// TODO: needed to make Lookup class work
	T* next_;
};


template <typename T>
class LinkedQueue
{
public:
	LinkedQueue() : first_(nullptr), pNext_(&first_) {}

	void addTail(T* item)
	{
		*pNext_ = item;
		pNext_ = reinterpret_cast<T**>(&item->next_);
		item->next_ = nullptr;
	}

	T* first() const
	{
		return first_;
	}

	void clear()
	{
		first_ = nullptr;
		pNext_ = &first_;
	}

private:
	T* first_;
	T** pNext_;
};


template <typename T>
class LinkedList
{
public:
	LinkedList() : first_(nullptr) {}

	void addHead(T* item)
	{
		item->next_ = first_;
		first_ = item;
	}

	T* first() const
	{
		return first_;
	}

	void clear()
	{
		first_ = nullptr;
	}

    class Iterator
    {
    public:
        Iterator(T* current) : current_(current) {}

        bool hasNext() const { return current_ != nullptr; }
        T* next() 
        {
            T* next = current_;
            current_ = reinterpret_cast<T*>(current_->next_);
            return next;
        }

    private:
        T* current_;
    };

	Iterator iter() const
	{
		return Iterator(first_);
	}

private:
	T* first_;
};




