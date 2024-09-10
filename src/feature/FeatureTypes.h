// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>

class FeatureTypes
{
public:
    static const uint32_t NODES =     0b00000000'00000101'00000000'00000101;
    static const uint32_t WAYS =      0b00000000'11110000'00000000'11110000;
    static const uint32_t RELATIONS = 0b00001111'00000000'00001111'00000000;
    static const uint32_t AREAS =     0b00001010'10100000'00001010'10100000;
    static const uint32_t NONAREA_WAYS = WAYS & (~AREAS);
    static const uint32_t NONAREA_RELATIONS = RELATIONS & (~AREAS);
    static const uint32_t ALL = NODES | WAYS | RELATIONS;
    static const uint32_t WAYNODE_FLAGGED = 0b00000000'11110101'00000000'00000000;
    static const uint32_t RELATION_MEMBERS = 0b00001100'11000100'00001100'11000100;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr FeatureTypes(uint32_t types) : types_(types) {}

    FeatureTypes operator|(FeatureTypes other) const noexcept
    {
        return {types_ | other.types_};
    }

    FeatureTypes& operator|=(const FeatureTypes& other) noexcept
    {
        types_ |= other.types_;
        return *this;
    }

    FeatureTypes operator&(FeatureTypes other) const noexcept
    {
        return {types_ & other.types_};
    }

    FeatureTypes operator&(uint32_t other) const noexcept
    {
        return {types_ & other};
    }

    FeatureTypes& operator&=(const FeatureTypes& other) noexcept
    {
        types_ &= other.types_;
        return *this;
    }

    operator uint32_t() const noexcept
    {
        return types_;
    }

    bool acceptFlags(int flags) const noexcept
    {
        // Unlike in Java, it is not a given that a bitwise shift
        // of a 32-bit type will only consider the lowest 5 bits
        // of the shift magnitude, so we'll apply a mask (0x1f)
        uint32_t typeFlag = 1 << ((flags >> 1) & 0x1f);
        return (types_ & typeFlag) != 0;
    }

private:
    uint32_t types_;
};
