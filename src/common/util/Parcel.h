// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <utility>

template <typename T>
class ParcelPtr
{
public:
	ParcelPtr() : p_(nullptr) {}
	ParcelPtr(T* p) : p_(p) {}
	~ParcelPtr() { clear(); }

	ParcelPtr(ParcelPtr&& other) noexcept :
		p_(other.p_)
	{
		other.p_ = nullptr;
	}

	void clear()
	{
		if (p_)
		{
			p_->~T();
			delete[] reinterpret_cast<uint8_t*>(p_);
			p_ = nullptr;
		}
	}

	// Move assignment operator
	ParcelPtr& operator=(ParcelPtr&& other) noexcept
	{
		std::swap(p_, other.p_);
		return *this;
	}

	operator bool() const noexcept
	{
		return p_ != nullptr;
	}

	ParcelPtr(const ParcelPtr&) = delete;
	ParcelPtr& operator=(const ParcelPtr&) = delete;

	T* get() const { return p_; }
	T& operator*() const { return *p_; }
	T* operator->() const { return p_; }

private:
	T* p_;
};

template <typename T>
class Parcel
{
public:
	uint8_t* data() { return reinterpret_cast<uint8_t*>(this) + sizeof(T); }
	size_t size() const { return size_; }
	void fill(const uint8_t* src)
	{
		memcpy(data(), src, size);
	}

protected:
	template <typename... Args>
	static ParcelPtr<T> create(size_t dataSize, Args&&... args)
	{
		uint8_t* bytes = new uint8_t[sizeof(T) + dataSize];
		T* parcel = new(bytes)T(std::forward<Args>(args)...);
		parcel->size_ = dataSize;
		return ParcelPtr<T>(parcel);
	}

	size_t size_;
};

