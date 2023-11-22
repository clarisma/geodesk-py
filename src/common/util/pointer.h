// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>

// TODO: This name may clash with some std:: classes

// TODO: should make little-Endian byte order explicit

class pointer
{
public:
	pointer() { ptr_ = nullptr; }
	pointer(const void* p) { ptr_ = reinterpret_cast<const char*>(p); }
	/*
	pointer(const char* p) { ptr_ = p; }
	pointer(const uint8_t* p) { ptr_ = reinterpret_cast<const char*>(p); }
	*/
	static pointer ofTagged(const void* p, uint64_t mask)
	{
		return pointer(reinterpret_cast<const char*>(
			reinterpret_cast<uint64_t>(p) & mask));
	}

	pointer follow() const
	{
		return pointer(ptr_ + getInt());
	}

	pointer followUnaligned() const
	{
		return pointer(ptr_ + getUnalignedInt());
	}

	pointer follow(int ofs) const
	{
		return pointer(ptr_ + getInt(ofs) + ofs);
	}

	pointer followUnaligned(int ofs) const
	{
		return pointer(ptr_ + getUnalignedInt(ofs) + ofs);
	}

	pointer followTagged(uint64_t mask) const
	{
		return pointer(ptr_ + (getInt() & mask));
	}

	int16_t getShort() const { return *reinterpret_cast<const int16_t*>(ptr_); }
	int16_t getShort(int ofs) const { return *reinterpret_cast<const int16_t*>(ptr_ + ofs); }
	uint16_t getUnsignedShort() const { return *reinterpret_cast<const uint16_t*>(ptr_); }
	uint16_t getUnsignedShort(int ofs) const { return *reinterpret_cast<const uint16_t*>(ptr_ + ofs); }
	int32_t getInt() const { return *reinterpret_cast<const int32_t*>(ptr_); }
	int32_t getInt(int ofs) const { return *reinterpret_cast<const int32_t*>(ptr_ + ofs); }
	int32_t getUnalignedInt() const { return *reinterpret_cast<const int32_t*>(ptr_); } // TODO: unaligned read
	int32_t getUnalignedInt(int ofs) const { return *reinterpret_cast<const int32_t*>(ptr_); } // TODO: unaligned read
	uint32_t getUnsignedInt() const { return *reinterpret_cast<const uint32_t*>(ptr_); }
	uint32_t getUnsignedInt(int ofs) const { return *reinterpret_cast<const uint32_t*>(ptr_ + ofs); }
	uint32_t getUnalignedUnsignedInt() const { return *reinterpret_cast<const uint32_t*>(ptr_); } // TODO: unaligned read
	int64_t getLong() const { return *reinterpret_cast<const int64_t*>(ptr_); }
	uint64_t getUnsignedLong() const { return *reinterpret_cast<const uint64_t*>(ptr_); }
	uint64_t getUnsignedLong(int ofs) const { return *reinterpret_cast<const uint64_t*>(ptr_ + ofs); }
	int64_t getUnalignedLong() const { return *reinterpret_cast<const int64_t*>(ptr_); } // TODO: alignment
	int64_t getUnalignedLong(int ofs) const { return *reinterpret_cast<const int64_t*>(ptr_ + ofs); } // TODO: alignment
	// double getDouble() const { return *reinterpret_cast<const double*>(ptr_); }

	pointer& operator+=(int32_t delta)
	{
		ptr_ += delta;
		return *this;
	}

	pointer& operator-=(int32_t delta)
	{
		ptr_ -= delta;
		return *this;
	}

	pointer operator+(int32_t delta) const { return pointer(ptr_ + delta); }
	pointer operator-(int32_t delta) const { return pointer(ptr_ - delta); }
	pointer operator+(uint32_t delta) const { return pointer(ptr_ + delta); }
	pointer operator-(uint32_t delta) const { return pointer(ptr_ - delta); }
	int32_t operator-(pointer other) const
	{ 
		return static_cast<int32_t>(ptr_ - other.ptr_);
	}

	int32_t operator-(const void* other) const 
	{ 
		return static_cast<int32_t>(ptr_ - reinterpret_cast<const char*>(other));
	}

	int32_t operator-(const uint8_t* other) const 
	{ 
		return static_cast<int32_t>(ptr_ - reinterpret_cast<const char*>(other)); 
	}

	pointer operator&(uint64_t mask) const 
	{ 
		return pointer(
			reinterpret_cast<const void*>(
				reinterpret_cast<uint64_t>(ptr_) & mask));
	}

	operator bool() const { return ptr_ != nullptr; }

	bool operator<(const pointer other)
	{
		return ptr_ < other.ptr_;
	}

	operator const uint8_t* () const
	{
		return reinterpret_cast<const uint8_t*>(ptr_);
	}

	operator const char* () const
	{
		return reinterpret_cast<const char*>(ptr_);
	}

	operator const void* () const
	{
		return reinterpret_cast<const void*>(ptr_);
	}

	uint64_t pointerAsULong() const
	{
		return reinterpret_cast<uint64_t>(ptr_);
	}

	const uint8_t* asBytePointer() const
	{
		return reinterpret_cast<const uint8_t*>(ptr_);
	}

	inline bool testAllFlags(int flags)
	{
		return (reinterpret_cast<std::uintptr_t>(ptr_) & flags) == flags;
	}

	inline bool testAnyFlags(int flags)
	{
		return (reinterpret_cast<std::uintptr_t>(ptr_) & flags) != 0;
	}

private:
	const char* ptr_;
};


