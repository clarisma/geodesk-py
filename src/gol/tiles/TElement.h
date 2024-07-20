// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>
// #include <common/alloc/Arena.h>
#include <common/data/Deduplicator.h>
#include <common/data/Linked.h>
#include <common/data/Lookup.h>

// TODO: In order to save 8 bytes per element, we use next_ (from Linked) to 
// chain items in the same bucket in an ElementDeduplicator
// However, we also use next_ for the chain of placed elements; this means
// that once we start placing elements, we can no longer look up elements in
// the ElementDeduplicator, becuase the hashmap chains are now invalid
// This should not be a problem, but need to document
// When placing elements, cannot assume that next_ is null!

template <typename T> class ElementDeduplicator;

class TElement : public Linked<TElement>
{
public:
	using Handle = int32_t;

	enum class Alignment : uint8_t
	{
		BYTE, WORD, DWORD, QWORD
	};

	enum class Type : unsigned int
	{
		UNKNOWN,
		STRING,
		TAGS,
		RELTABLE,
		FEATURE,
		WAY_BODY,
		RELATION_BODY,
		INDEX,
		TRUNK,
		LEAF
	};

	TElement(Type type, Handle handle, uint32_t size, Alignment alignment, int anchor = 0) :
		Linked(nullptr),
		location_(0), 
		size_(size),
		alignment_(static_cast<unsigned int>(alignment)),
		handle_(handle),
		type_(type),
		isLast_(false),
		isDeleted_(false),
		anchor_(anchor)
	{
	}

	template<typename T>
	static T* cast(TElement* e)
	{
		T* casted = reinterpret_cast<T*>(e);
		assert(!casted || casted->type() == T::TYPE);
		return casted;
	}

	Type type() const { return type_; }
	TElement* next() const { return next_; }
	void setNext(TElement* next) { next_ = next; }
	int32_t location() const { return location_; }
	void setLocation(int32_t location) { location_ = location; }
	Handle handle() const { return handle_; }
	uint32_t size() const { return size_; }
	void setAlignment(Alignment alignment) 
	{ 
		alignment_ = static_cast<unsigned int>(alignment); 
	}
	int32_t alignedLocation(int32_t loc)
	{
		int add = (1 << alignment_) - 1;
		int32_t mask = 0xffff'ffff << alignment_;
		return (loc + add) & mask;
	}
	uint32_t anchor() const { return anchor_; }
	bool isLast() const { return isLast_; }
	void setLast(bool last) { isLast_ = last; }
	void setAnchor(uint32_t anchor) { anchor_ = anchor; }

protected:
	void setSize(uint32_t size) { size_ = size; }

private:
	int32_t location_;
	unsigned int alignment_ :  2;
	unsigned int size_      : 30;
	Handle handle_;
		// TODO: move handle to TReferencedElement?
		// No, need it here because of alignment
		// If we were to take it out, this class only has 3 32-bit values,
		// which would leave a gap
	Type type_              :  6;
	bool isLast_            :  1;
	bool isDeleted_         :  1;
	unsigned int anchor_    : 24;
		// If we could limit the anchor to 64K, we could free
		// up 8 bits for flags
};

class TReferencedElement : public TElement
{
public:
	TReferencedElement(Type type, Handle handle, uint32_t size,
		Alignment alignment, int anchor) :
		TElement(type, handle, size, alignment, anchor),
		nextByHandle_(nullptr)
	{
	}

private:
	TReferencedElement* nextByHandle_;

	friend class LookupByHandle;
};

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
	uint8_t* mutableData() const { return data_; }
	uint32_t hash() const { return hash_; }

	void write(uint8_t* p) const
	{
		memcpy(p, data_, size());
	}

	virtual bool operator<(const TSharedElement& other) const
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
	uint8_t* data_;
	uint32_t hash_;
	unsigned int users_ : 24;
	unsigned int category_ : 8;
};



class LookupByHandle : public Lookup<LookupByHandle, TReferencedElement>
{
public:
	static uint64_t getId(TReferencedElement* element)
	{
		return element->handle();
	}

	static TReferencedElement** next(TReferencedElement* elem)
	{
		return &elem->nextByHandle_;
	}
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
