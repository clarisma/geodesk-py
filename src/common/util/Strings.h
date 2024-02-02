// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>


namespace Strings
{
    inline size_t hash(const char* str, size_t length)
    {
        size_t hash = 5381;  // Initial value for djb2 algorithm
        for (size_t i = 0; i < length; ++i)
        {
            // hash * 33 + c
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(str[i]);
        }
        return hash;
    }
}

class ShortString
{
public:
    void init(const char* chars, size_t len)
    {
        assert(len < (1 << 16));
        len_ = static_cast<uint16_t>(len);
        memcpy(chars_, chars, len);
    }

    uint32_t length() const { return len_; }

    const char* data() const { return chars_; }

    uint32_t totalSize() const
    {
        return len_ + sizeof(len_);
    }

    template<int Alignment>
    uint32_t alignedTotalSize() const
    {
        static_assert(Alignment && !(Alignment & (Alignment - 1)), "Alignment must be a power of 2");
        return (totalSize() + Alignment - 1) & ~(Alignment - 1);
    }

    bool operator==(const ShortString& other) const 
    {
        if (length() != other.length()) return false;
        return memcmp(chars_, other.chars_, length()) == 0;
    }

private:
    uint16_t len_;
    char chars_[1];
};

class ShortVarString
{
public:
    uint32_t length() const 
    { 
        uint8_t len = bytes_[0];
        return (len & 0x80) ? ((static_cast<uint32_t>(bytes_[1]) << 7) | (len & 0x7f)) : len;
    }

    const char* data() const 
    { 
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return reinterpret_cast<const char*>(&bytes_[ofs]); 
    }

    uint32_t totalSize() const
    {
        uint32_t lenSize = (bytes_[0] >> 7) + 1;
        return length() + lenSize;
    }

    template<int Alignment>
    uint32_t alignedTotalSize() const
    {
        static_assert(Alignment && !(Alignment & (Alignment - 1)), "Alignment must be a power of 2");
        return (totalSize() + Alignment - 1) & ~(Alignment - 1);
    }

    bool operator==(const ShortVarString& other) const
    {
        uint32_t len = length();
        if (len != other.length()) return false;
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return memcmp(&bytes_[ofs], &other.bytes_[ofs], len) == 0;
    }

    std::string_view toStringView() const
    {
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return std::string_view(reinterpret_cast<const char*>(&bytes_[ofs]), length());
    }

private:
    uint8_t bytes_[1];
};


class TinyString
{
public:
    uint32_t length() const
    {
        return bytes_[0];
    }

    const char* data() const
    {
        return reinterpret_cast<const char*>(&bytes_[1]);
    }

    uint32_t totalSize() const
    {
        return length() + 1;
    }

    template<int Alignment>
    uint32_t alignedTotalSize() const
    {
        static_assert(Alignment && !(Alignment & (Alignment - 1)), "Alignment must be a power of 2");
        return (totalSize() + Alignment - 1) & ~(Alignment - 1);
    }

    bool operator==(const TinyString& other) const
    {
        uint32_t len = length();
        if (len != other.length()) return false;
        return memcmp(&bytes_[1], &other.bytes_[1], len) == 0;
    }

    std::string_view toStringView() const
    {
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return std::string_view(reinterpret_cast<const char*>(&bytes_[ofs]), length());
    }

private:
    uint8_t bytes_[1];
};


class HashedShortString
{
public:
    void init(uint32_t hash, const char* chars, size_t len)
    {
        hash_ = hash;
        str_.init(chars, len);
    }

    void init(const char* chars, size_t len)
    {
        init(Strings::hash(chars, len), chars, len);
    }

    uint32_t hash() const { return hash_; }

    uint32_t length() const { return str_.length(); }

    uint32_t totalSize() const
    {
        return str_.totalSize() + sizeof(hash_);
    }

    template<int Alignment>
    uint32_t alignedTotalSize() const
    {
        static_assert(Alignment && !(Alignment & (Alignment - 1)), "Alignment must be a power of 2");
        return (totalSize() + Alignment - 1) & ~(Alignment - 1);
    }

    bool operator==(const HashedShortString& other) const
    {
        // TODO: Could combine hash & length check into one comparison
        // by reading a uint64_t and masking off the lower 48 bits
        // But check for alignment issues and possible overread
        if (hash_ != other.hash_) return false;
        return str_ == other.str_;
    }

private:
    uint32_t hash_;
    ShortString str_;
};

