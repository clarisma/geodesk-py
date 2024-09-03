// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>

template <typename T>
class Span
{
public:
    using value_type = T;

    Span() : p_(nullptr), size_(0) {}
    Span(T* p, size_t size) : p_(p), size_(size) {}
    Span(T* begin, T* end) : p_(begin), size_(end - begin) {}

    template<size_t N>
    Span(T(&arr)[N]) : p_(arr), size_(N) {}

    template<typename Container>
    Span(Container& container) : 
        p_(reinterpret_cast<T*>(container.data())), 
            // TODO: This cast is dangerous, needed for now for BufferWriter
        size_(container.size()) {}

    size_t size() const { return size_; }
    bool isEmpty() const { return size_ == 0; }

    // Access elements
    T& operator[](size_t index) const 
    { 
        assert(index < size_);
        return p_[index]; 
    }

    void removeAt(size_t index)
    {
        assert(index < size_);
        std::move(p_ + index + 1, p_ + size_, p_ + index);
        --size_;
    }

    T* data() const { return p_; }

    // Iterator support
    T* begin() const { return p_; }

    T* end() const { return p_ + size_; }

private:
    T* p_;             // Pointer to the start of the range
    std::size_t size_; // Number of elements in the span
};

using ByteSpan = Span<const uint8_t>;

