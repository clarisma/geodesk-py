// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>


// TODO: ensure empty string can be safely deoded without buffer overrun
//  or enforce rule that second byte of a zero-length string must
//  always be readable

///
///  A string with a maximum length of 2^14-1 bytes.
///  The string length is stored as a 14-bit varint ahead of the
///  character data (NOT null-terminated)
///
class ShortVarString
{
public:
    // TODO: Fix; this class should never be instantiated via
    //  its constructor, it is merely a wrapper around an
    //  array of bytes
    ShortVarString() {} // do nothing
    ShortVarString(const ShortVarString&) = delete;
    ShortVarString& operator=(const ShortVarString&) = delete;

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

    bool isEmpty() const noexcept
    {
        return bytes_[0] == 0;
    }

    // To conform with other string classes -- don't confuse with totalSize(), which includes
    // the length byte(s), whereas size() is the actual string length only
    size_t size() const noexcept
    {
        return length();
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

    bool operator!=(const ShortVarString& other) const noexcept
    {
        return !(*this == other);
    }

    bool operator==(const std::string_view& other) const noexcept
    {
        uint32_t len = length();
        if (len != other.length()) return false;
        uint32_t ofs = (bytes_[0] >> 7) + 1;
        return memcmp(&bytes_[ofs], other.data(), len) == 0;
    }

    bool operator!=(const std::string_view& other) const noexcept
    {
        return !(*this == other);
    }

    bool equals(const char* str, size_t len) const
    {
        if (length() != len) return false;
        return std::memcmp(data(), str, len) == 0;
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

    static bool compare(const ShortVarString* a, const ShortVarString* b)
    {
        uint32_t ofs1 = (a->bytes_[0] >> 7) + 1;
        uint32_t ofs2 = (b->bytes_[0] >> 7) + 1;
        uint32_t len1 = a->length();
        uint32_t len2 = b->length();
        uint32_t commonLen = std::min(len1, len2);
        int res = memcmp(a->bytes_ + ofs1, b->bytes_ + ofs2, commonLen);
        if (res == 0)
        {
            return len1 < len2;
        }
        return res < 0;
    }

    const uint8_t* rawBytes() const { return bytes_; }

private:
    uint8_t bytes_[1];
};
