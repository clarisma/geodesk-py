#pragma once

#include <cassert>
#include <cstdint>
// #include <common/alloc/Arena.h>
#include <common/data/Deduplicator.h>
#include <common/data/Linked.h>
#include <common/data/Lookup.h>

template <typename T> class ElementDeduplicator;

class TElement : public Linked<TElement>
{
public:
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

	TElement(Type type, int32_t location, uint32_t size, Alignment alignment, int anchor = 0) :
		Linked(nullptr),
		location_(0), 
		size_(size),
		alignment_(static_cast<unsigned int>(alignment)),
		oldLocation_(location),
		type_(type),
		isLast_(false),
		isDeleted_(false),
		anchor_(anchor)
	{
	}

	Type type() const { return type_; }
	TElement* next() const { return next_; }
	void setNext(TElement* next) { next_ = next; }
	int32_t location() const { return location_; }
	void setLocation(int32_t location) { location_ = location; }
	int32_t oldLocation() const { return oldLocation_; }
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

protected:
	void setSize(uint32_t size) { size_ = size; }

private:
	int32_t location_;
	unsigned int alignment_ :  2;
	unsigned int size_      : 30;
	int32_t oldLocation_;
	Type type_              :  6;
	bool isLast_            :  1;
	bool isDeleted_         :  1;
	unsigned int anchor_    : 24;
};

class TReferencedElement : public TElement
{
public:
	TReferencedElement(Type  type, int32_t location, uint32_t size,
		Alignment alignment, int anchor) :
		TElement(type, location, size, alignment, anchor), 
		nextByLocation_(nullptr)
	{
	}

private:
	TReferencedElement* nextByLocation_;

	friend class LookupByLocation;
};


class TSharedElement : public TReferencedElement
{
public:
	TSharedElement(Type type, int32_t location, const uint8_t* data, 
		uint32_t size, Alignment alignment, int anchor = 0) :
		TReferencedElement(type, location, size, alignment, anchor),
		data_(data)
	{
	}

	const uint8_t* data() const { return data_; }
	void write(uint8_t* p) const
	{
		memcpy(p, data_, size());
	}

	static const int MIN_COMMON_USAGE = 4;

protected:
	const uint8_t* data_;
	// uint32_t usage_;
	// uint32_t extra_;		// can be used by subclasses

	// friend class ElementDeduplicator<TSharedElement>;
};



class LookupByLocation : public Lookup<LookupByLocation, TReferencedElement>
{
public:
	static uint64_t getId(TReferencedElement* element)
	{
		return element->oldLocation();
	}

	static TReferencedElement** next(TReferencedElement* elem)
	{
		return &elem->nextByLocation_;
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
