// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <common/text/Format.h>

// internal; needed by GOL

class TipDelta
{
public:
    constexpr TipDelta() : delta_(0) {}
    constexpr TipDelta(int32_t delta) : delta_(delta) {}

    operator int32_t() const { return delta_; }

    bool isWide()
    {
        // 15 bits signed
        return (delta_ << 17 >> 17) != delta_;
    }

private:
    int32_t delta_;
};

class Tip
{
public:
    constexpr Tip() : tip_(0) {}
    constexpr explicit Tip(uint32_t tip) : tip_(tip)
    {
        assert(tip <= MAX_TIP_VALUE);
    }

    static const uint32_t MAX_TIP_VALUE = 0xffffff;

    bool isNull() const
    {
        return tip_ == 0;
    }

    operator uint32_t() const
    {
        return tip_;
    }

    Tip& operator+=(TipDelta delta)
    {
        tip_ += delta;
        return *this;
    }

    bool operator==(const Tip& other) const
    {
        return tip_ == other.tip_;
    }

    TipDelta operator-(Tip other) const noexcept
    {
        return TipDelta(tip_ - other.tip_);
    }

    char* format(char* buf) const
    {
        Format::hexUpper(buf, tip_, 6);
        return buf;
    }

    std::string toString() const
    {
        char buf[8];
        return std::string(format(buf));
    }

private:
    uint32_t tip_;
};

template<typename Stream>
Stream& operator<<(Stream& out, const Tip& tip)
{
    char buf[8];
    tip.format(buf);
    out.write(buf, 6);
    return out;
}

namespace std
{
template<>
struct hash<Tip>
{
    size_t operator()(const Tip& tile) const
    {
        return std::hash<uint32_t>()(tile);
    }
};
}
