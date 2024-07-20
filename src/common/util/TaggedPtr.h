// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <cstdint>
#include <iostream>
#include <type_traits>

template <typename T, int N>
class TaggedPtr
{
    static_assert(N > 0 && N < 4, "Flags bits must be 1,2 or 3");

public:
    // Constructor
    TaggedPtr(T* ptr = nullptr, uintptr_t flags = 0) noexcept :
        data_(reinterpret_cast<uintptr_t>(ptr) | (flags & flag_mask()))
    {
    }

    // Get the pointer with flags
    T* ptr() const noexcept
    {
        return reinterpret_cast<T*>(data_ & ~flag_mask());
    }

    // Get the flags
    int flags() const noexcept
    {
        return data_ & flag_mask();
    }

    // Implicit conversion to a regular pointer
    operator T* () const noexcept
    {
        return ptr();
    }

    // Set the flags
    void setFlags(uintptr_t flags) noexcept
    {
        data_ = reinterpret_cast<uintptr_t>(ptr()) | (flags & flagMask());
    }

private:
    uintptr_t data_;

    // Calculate the flag mask
    static constexpr uintptr_t flagMask() noexcept
    {
        return (1UL << N) - 1;
    }
};
