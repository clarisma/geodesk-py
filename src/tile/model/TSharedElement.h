// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TReferencedElement.h"

// TODO: Fix sorting of elements, don't use anything virtual
// because it bloats the class

// TODO: Would be useful to mark TTagTable to see if it can be quickly compared
// bytewise since it does not contain string pointers
class TSharedElement : public TReferencedElement
{
public:
	TSharedElement(Type type, Handle handle, const uint8_t* data,
		uint32_t size, Alignment alignment, uint32_t hash, int anchor = 0) :
		TReferencedElement(type, handle, size, alignment, anchor),
		data_(data), hash_(hash), users_(0), category_(0)
	{
	}

	const uint8_t* data() const { return data_; }
	uint8_t* mutableData() const { return const_cast<uint8_t*>(data_); }
	uint32_t hash() const { return hash_; }

	void write(uint8_t* p) const
	{
		memcpy(p, data_, size());
	}

	bool operator<(const TSharedElement& other) const
	{
		uint32_t commonSize = std::min(size(), other.size());
		int res = memcmp(data(), other.data(), commonSize);
		if (res == 0)
		{
			return size() < other.size();
		}
		return res < 0;
	}

	int users() const { return users_; }
	void addUser() { users_++; }
	int category() const { return category_; }
	void setCategory(int category) { category_ = category; }

	static const int MIN_COMMON_USAGE = 4;

protected:
	const uint8_t* data_;
	uint32_t hash_;
	unsigned int users_ : 24;
	unsigned int category_ : 8;
};


template<typename T>
class ElementDeduplicator : public Deduplicator<ElementDeduplicator<T>, T>
{
public:
	static const void* data(T* item)
	{
		return item->data();
	}

	static int length(T* item)
	{
		return item->size();
	}

	static T** next(T* elem)
	{
		return reinterpret_cast<T**>(&elem->next_);
	}
};
