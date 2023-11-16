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
		sizeAndAlignment_((size << 2) | static_cast<uint32_t>(alignment)) 
	{
	}

	int32_t location() const { return location_; }
	void setLocation(int32_t location) { location_ = location; }
	uint32_t size() const { return sizeAndAlignment_ >> 2; }
	int32_t alignedLocation(int32_t loc)
	{
		int alignment = (sizeAndAlignment_ & 3);
		int add = (1 << alignment) - 1;
		int32_t mask = 0xffff'ffff << alignment;
		return (loc + add) & mask;
	}

private:
	int32_t location_;
	uint32_t sizeAndAlignment_;
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

public:								// workaround for template access
	TSharedElement* nextByType_;
private:
	const uint8_t* data_;
	// uint32_t usage_;
	// uint32_t extra_;		// can be used by subclasses

	// friend class ElementDeduplicator<TSharedElement>;
};



class LookupByLocation : public Lookup<LookupByLocation, TIndexedElement>
{
public:
	static int64_t getId(TIndexedElement* element)
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
