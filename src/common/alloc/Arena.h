// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <new>
#include <common/data/Span.h>

// TODO: make it part of Arena contract NOT to alloc memory in constructor
// (defer until actual objects are alloc'ed); this makes it easier to use in 
// constructors

static_assert(nullptr == 0, "nullptr is not represented as 0!");

class Arena
{
public:
	enum class GrowthPolicy
	{
		DOUBLE = 0,
		GROW_50_PERCENT = 1,
		GROW_25_PERCENT = 2,
		SAME_SIZE = 63,
	};

	Arena(size_t chunkSize = 4096, GrowthPolicy growth = GrowthPolicy::DOUBLE) :
		current_(nullptr),
		p_(nullptr),
		end_(nullptr),
		nextSize_(chunkSize),
		intialSizeAndPolicy_((chunkSize << 8) | (uint8_t)growth)
	{
	}

	uint64_t initialSize() const
	{
		return intialSizeAndPolicy_ >> 8;
	}

	uint64_t nextSize(uint64_t currentSize) const
	{
		return currentSize + (currentSize >> (intialSizeAndPolicy_ & 0xff));
	}
	
	~Arena()
	{
		Chunk* chunk = current_;
		while (chunk)
		{
			Chunk* next = chunk->next;
			delete[] reinterpret_cast<uint8_t*>(chunk);
			chunk = next;
		}
	}

	void clear()
	{
		// TODO: If we could make sure that the oldest chunk in the chain
		// is <initialSize> (i.e. not a whale, which may be smaller), we
		// could keep the oldest chunk instead of freeing all chunks

		Chunk* chunk = current_;
		while (chunk)
		{
			Chunk* next = chunk->next;
			delete[] reinterpret_cast<uint8_t*>(chunk);
			chunk = next;
		}
		nextSize_ = initialSize();
		current_ = nullptr;
		p_ = nullptr;
		end_ = nullptr;
	}

	uint8_t* alloc(size_t size, int align)
	{
		p_ += (align - (reinterpret_cast<uintptr_t>(p_) & (align - 1))) & (align - 1);
		if (p_ + size > end_) allocChunk(size);
		uint8_t* pStart = p_;
		p_ += size;
		return pStart;
	}

	template <typename T>
	T* alloc()
	{
		return reinterpret_cast<T*>(alloc(sizeof(T), alignof(T)));
	}

	template<typename T>
	T* create()
	{
		T* ptr = alloc<T>();
		new(ptr) T();
		return ptr;
	}

	template <typename T, typename... Args>
	T* create(Args&&... args)
	{
		T* p = alloc<T>();
		new(p)T(std::forward<Args>(args)...);
		return p;
	}

	template <typename T>
	T* allocArray(size_t count)
	{
		return reinterpret_cast<T*>(alloc(count * sizeof(T), alignof(T)));
	}

	template <typename T>
	Span<typename std::remove_const<T>::type> allocCopy(Span<T> span)
	{
		// Use std::remove_const to ensure we allocate the correct type
		using NonConstT = typename std::remove_const<T>::type;

		NonConstT* p = allocArray<NonConstT>(span.size());
		memcpy(p, span.data(), span.size() * sizeof(T));
		return Span<NonConstT>(p, span.size());
	}

	template <typename T>
	T* allocWithExplicitSize(size_t size)
	{
		return reinterpret_cast<T*>(alloc(size, alignof(T)));
	}

	/**
	 * Trims back the last allocated element by the specified number of bytes.
	 * 
	 * WARNING!
	 * This is a low-level function that requires much care.
	 * This function will corrupt memory if other allocations took place
	 * after the object to be trimmed has been allocated.
	 */
	void reduceLastAlloc(size_t reduceBy)
	{
		p_ -= reduceBy;
		assert(p_ >= reinterpret_cast<uint8_t*>(current_) + sizeof(Chunk));
	}

	/**
	 * Removes the last allocated element.
	 *
	 * WARNING!
	 * This is a low-level function that requires much care.
	 * This function will corrupt memory if other allocations took place
	 * after the object to be trimmed has been allocated.
	 */
	void freeLastAlloc(void *data)
	{
		p_ = reinterpret_cast<uint8_t*>(data);
		assert(p_ >= reinterpret_cast<uint8_t*>(current_) + sizeof(Chunk));
	}

private:
	struct Chunk
	{
		Chunk* next;
	};

	void allocChunk(size_t size);

	Chunk* current_;
	uint8_t* p_;
	uint8_t* end_;
	uint64_t nextSize_;
	uint64_t intialSizeAndPolicy_;
};


