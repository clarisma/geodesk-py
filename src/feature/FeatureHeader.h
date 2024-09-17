// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>

class FeatureHeader
{
public:
    FeatureHeader(uint64_t bits) : bits_(bits) {}

    // TODO: This will change in 2.0
    static FeatureHeader forTypeAndId(int type, uint64_t id)
    {
        uint64_t hi = (((id >> 32) << 8) | (type << 3));
        uint64_t lo = id << 32;
        return FeatureHeader(hi | lo);
    }

    uint64_t bits() const noexcept { return bits_; }

    // TODO: This will change in 2.0
    uint64_t id() const noexcept
    {
        uint32_t hi = static_cast< uint32_t>(bits_) >> 8;
        uint32_t lo = static_cast<uint32_t>(bits_ >> 32);
        return (static_cast<uint64_t>(hi) << 32) | lo;
    }

    // TODO: This will change in 2.0
    int flags() const noexcept
    {
        return static_cast<int>(bits_);
    }

    int typeCode() const
    {
        return static_cast<int>(bits_ >> 3) & 3;
    }

private:
    uint64_t bits_;
};
