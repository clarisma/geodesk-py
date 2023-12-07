#pragma once
#include <cassert>
#include <cstdint>

template <typename T>
class Linked
{
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

private:
	T* first_;
	T** pNext_;
};



