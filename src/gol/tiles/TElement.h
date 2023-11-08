#pragma once

#include <cassert>
#include <cstdint>
// #include <common/alloc/Arena.h>
#include <common/data/Deduplicator.h>
#include <common/data/Lookup.h>

class TElement
{
public:
	class SizeAndAlignment
	{
	public:
		SizeAndAlignment(uint32_t size, uint32_t alignment)
		{
			assert(alignment < 4);
			value_ = (size << 2) | alignment;
		}

		uint32_t size() const { return value_ >> 2;  }

	private:
		uint32_t value_;
	};

	static SizeAndAlignment unaligned(uint32_t size)
	{
		return SizeAndAlignment(size, 0);
	}

	static SizeAndAlignment aligned2(uint32_t size)
	{
		return SizeAndAlignment(size, 1);	// 2 == 1 << 1
	}

	static SizeAndAlignment aligned4(uint32_t size)
	{
		return SizeAndAlignment(size, 2);	// 4 == 1 << 2
	}

	TElement(int32_t location, SizeAndAlignment sizeAndAlignment) :
		location_(location), sizeAndAlignment_(sizeAndAlignment) {}
	int32_t location() const { return location_; }
	void setLocation(int32_t location) { location_ = location; }
	uint32_t size() const { return sizeAndAlignment_.size(); }

private:
	int32_t location_;
	SizeAndAlignment sizeAndAlignment_;
};

class TIndexedElement : public TElement
{
public:
	TIndexedElement(int32_t location, SizeAndAlignment sizeAndAlignment) :
		TElement(location, sizeAndAlignment), nextByLocation_(nullptr) {}

private:
	TIndexedElement* nextByLocation_;

	friend class LookupByLocation;
};

class TSharedElement : public TIndexedElement
{
public:
	TSharedElement(int32_t location, const uint8_t* data, SizeAndAlignment sizeAndAlignment) :
		TIndexedElement(location, sizeAndAlignment), 
		data_(data),
		nextByType_(nullptr) 
	{
	}

	const uint8_t* data() const { return data_; }
	void write(uint8_t* p)
	{
		memcpy(p, data_, size());
	}

private:
	TSharedElement* nextByType_;
	const uint8_t* data_;
};



class LookupByLocation : public Lookup<LookupByLocation, TIndexedElement>
{
public:
	/*
	void init(Arena& arena, size_t tableSize)
	{
		init(arena.allocArray<TIndexedElement*>(tableSize), tableSize);
	}
	*/

protected:
	int64_t getId(TIndexedElement* element)
	{
		return element->location();
	}

	TIndexedElement** next(TIndexedElement* elem)
	{
		return &elem->nextByLocation_;
	}
};

template<typename T>
class ElementDeduplicator : public Deduplicator<ElementDeduplicator<T>, T>
{
protected:
	const void* data(T* item)
	{
		return item->data();
	}

	int length(T* item)
	{
		return item->size();
	}

	TIndexedElement** next(TIndexedElement* elem)
	{
		return &elem->nextByLocation_;
	}
};
