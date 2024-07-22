// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <cstring>
#include <string_view>


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

    template <typename S>
    inline size_t hash(const S& str)
    {
        return hash(str.data(), str.length());
    }
}

/**
 * A string with a maximum length of 2^16-1 bytes.
 * The string length is stored as a uint16_t.
 */
class ShortString
{
public:
    void init(const char* chars, size_t len)
    {
        assert(len < (1 << 16));
        len_ = static_cast<uint16_t>(len);
        memcpy(chars_, chars, len);
    }

    uint32_t length() const noexcept { return len_; }

    const char* data() const noexcept { return chars_; }

    static uint32_t totalSize(uint32_t len) noexcept
    {
        return len + sizeof(len_);
    }

    uint32_t totalSize() const noexcept
    {
        return totalSize(len_);
    }

    bool operator==(const ShortString& other) const noexcept
    {
        if (length() != other.length()) return false;
        return memcmp(chars_, other.chars_, length()) == 0;
    }

private:
    uint16_t len_;
    char chars_[1];
};

/**
 * A string with a maximum length of 2^14-1 bytes.
 * The string length is stored as a 14-bit varint.
 */
class ShortVarString
{
public:
    void init(const ShortVarString* other)
    {
        init(other->data(), other->length());
    }

    /*
    static constexpr const ShortVarString& cast(const uint8_t* bytes)
    {
        return *reinterpret_cast<const ShortVarString*>(bytes);
    }
    */

    void init(const char* chars, size_t len)
    {
        int n;
        if (len < 128)
        {
            bytes_[0] = static_cast<uint8_t>(len);
            n = 1;
        }
        else
        {
            assert(len < (1 << 14));
            bytes_[0] = static_cast<uint8_t>((len & 0x7F) | 0x80);
            bytes_[1] = static_cast<uint8_t>(len >> 7);
            n = 2;
        }
        memcpy(&bytes_[n], chars, len);
    }

    uint32_t length() const noexcept
    { 
        uint8_t len = bytes_[0];
        return (len & 0x80) ? ((static_cast<uint32_t>(bytes_[1]) << 7) | (len & 0x7f)) : len;
    }

    const char* data() const noexcept
    { 
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return reinterpret_cast<const char*>(&bytes_[ofs]); 
    }

    static uint32_t totalSize(uint32_t len) noexcept
    {
        return len + (len >= 128 ? 2 : 1);
    }

    uint32_t totalSize() const noexcept
    {
        uint32_t lenSize = (bytes_[0] >> 7) + 1;
        return length() + lenSize;
    }

    bool operator==(const ShortVarString& other) const noexcept
    {
        uint32_t len = length();
        if (len != other.length()) return false;
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return memcmp(&bytes_[ofs], &other.bytes_[ofs], len) == 0;
    }

    bool operator==(const std::string_view& other) const noexcept
    {
        uint32_t len = length();
        if (len != other.length()) return false;
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return memcmp(&bytes_[ofs], other.data(), len) == 0;
    }

    std::string toString() const noexcept
    {
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return std::string(reinterpret_cast<const char*>(&bytes_[ofs]), length());
    }

    std::string_view toStringView() const noexcept
    {
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return std::string_view(reinterpret_cast<const char*>(&bytes_[ofs]), length());
    }

private:
    uint8_t bytes_[1];
};


/**
 * A string with a maximum length of 255 bytes.
 * The string length is stored as a single byte.
 */
class TinyString
{
public:
    void init(const char* chars, size_t len)
    {
        assert(len < 256);
        bytes_[0] = static_cast<uint8_t>(len);
        memcpy(&bytes_[1], chars, len);
    }

    uint32_t length() const noexcept
    {
        return bytes_[0];
    }

    const char* data() const noexcept
    {
        return reinterpret_cast<const char*>(&bytes_[1]);
    }

    static constexpr uint32_t totalSize(uint32_t len) noexcept
    {
        return len + 1;
    }

    uint32_t totalSize() const noexcept
    {
        return totalSize(length());
    }

    bool operator==(const TinyString& other) const noexcept
    {
        uint32_t len = length();
        if (len != other.length()) return false;
        return memcmp(&bytes_[1], &other.bytes_[1], len) == 0;
    }

    std::string_view toStringView() const noexcept
    {
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return std::string_view(reinterpret_cast<const char*>(&bytes_[ofs]), length());
    }

private:
    uint8_t bytes_[1];
};


template <std::size_t N>
class TinyStringConstant
{
public:
    constexpr TinyStringConstant(const char(&str)[N])
    {
        init(str, N - 1);
    }

private:
    uint8_t chars_[N - 1];
};

template <std::size_t N> 
TinyStringConstant(const char(&)[N]) -> TinyStringConstant<N + 1>;
