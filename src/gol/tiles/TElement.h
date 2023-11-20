#pragma once

#include <cassert>
#include <cstdint>
// #include <common/alloc/Arena.h>
#include <common/data/Deduplicator.h>
#include <common/data/Lookup.h>

template <typename T> class ElementDeduplicator;

class TElement
{
public:
	enum class Alignment : uint8_t
	{
		BYTE, WORD, DWORD, QWORD
	};

	TElement(int32_t location, uint32_t size, Alignment alignment) :
		location_(location), 
		size_(size),
		alignment_(static_cast<unsigned int>(alignment))
	{
	}

	int32_t location() const { return location_; }
	void setLocation(int32_t location) { location_ = location; }
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

private:
	int32_t location_;
	unsigned int alignment_ :  2;
	unsigned int size_      : 30;
	// TODO: Need oldLocation_, type/anchor, next_
};

class TIndexedElement : public TElement
{
public:
	TIndexedElement(int32_t location, uint32_t size, Alignment alignment) :
		TElement(location, size, alignment), 
		nextByLocation_(nullptr)
	{
	}

private:
	TIndexedElement* nextByLocation_;

	friend class LookupByLocation;
};


class TSharedElement : public TIndexedElement
{
public:
	TSharedElement(int32_t location, const uint8_t* data, uint32_t size, Alignment alignment) :
		TIndexedElement(location, size, alignment), 
		data_(data),
		nextByType_(nullptr)
	{
	}

	const uint8_t* data() const { return data_; }
	void write(uint8_t* p) const
	{
		memcpy(p, data_, size());
	}

	static const int MIN_COMMON_USAGE = 4;

public:								// workaround for template access
	TSharedElement* nextByType_;
protected:
	const uint8_t* data_;
	// uint32_t usage_;
	// uint32_t extra_;		// can be used by subclasses

	// friend class ElementDeduplicator<TSharedElement>;
};



class LookupByLocation : public Lookup<LookupByLocation, TIndexedElement>
{
public:
	static uint64_t getId(TIndexedElement* element)
	{
		return element->location();
	}

	static TIndexedElement** next(TIndexedElement* elem)
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
		return reinterpret_cast<T**>(&elem->nextByType_);
	}
};
